#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/primitive.hpp"
#include "dort/lighting.hpp"
#include "dort/low_discrepancy.hpp"
#include "dort/sppm_renderer.hpp"

namespace dort {
  void SppmRenderer::render(CtxG& ctx) {
    this->light_distrib = compute_light_distrib(*this->scene);
    this->pixel_pos_idx = this->sampler->request_sample_2d();

    float radius = this->initial_radius;
    for(uint32_t i = 0; i < this->iteration_count; ++i) {
      this->iteration(ctx, *this->film, *this->sampler, radius);
      radius *= sqrt((float(i) + this->alpha) / (float(i) + 1.f));
    }
  }

  void SppmRenderer::iteration(CtxG& ctx, Film& film,
      Sampler& sampler, float radius) const 
  {
    PhotonMap photon_map = this->compute_photon_map(ctx, sampler.rng.split());
    for(uint32_t x = 0; x < film.x_res; ++x) {
      for(uint32_t y = 0; y < film.y_res; ++y) {
        sampler.start_pixel();
        for(uint32_t s = 0; s < sampler.samples_per_pixel; ++s) {
          sampler.start_pixel_sample();
          this->gather_pixel(film, sampler, photon_map, radius, x, y, s);
        }
      }
    }
  }

  void SppmRenderer::gather_pixel(Film& film, Sampler& sampler,
      const PhotonMap& photon_map, float radius,
      uint32_t x, uint32_t y, uint32_t) const
  {
    Vec2 pixel_pos = sampler.get_sample_2d(this->pixel_pos_idx);
    Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
    Vec2 film_res = Vec2(float(film.x_res), float(film.y_res));
    Ray ray(this->scene->camera->cast_ray(film_res, film_pos));

    Spectrum radiance = this->gather_ray(ray, sampler, photon_map, radius);
    film.add_sample(film_pos, radiance);
  }

  Spectrum SppmRenderer::gather_ray(Ray ray, Sampler& sampler,
      const PhotonMap& photon_map, float radius) const
  {
    Spectrum radiance(0.f);
    Spectrum weight(1.f);

    for(uint32_t depth = 0; depth < this->max_depth; ++depth) {
      Intersection isect;
      if(!this->scene->intersect(ray, isect)) {
        for(const auto& light: this->scene->lights) {
          if(light->flags & LIGHT_BACKGROUND) {
            radiance += weight * light->background_radiance(ray);
          }
        }
        break;
      }

      Spectrum emitted = isect.emitted_radiance(-ray.dir);
      assert(is_finite(emitted) && is_nonnegative(emitted));
      radiance += weight * emitted;

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo = normalize(-ray.dir);
      geom.ray_epsilon = isect.ray_epsilon;

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      BxdfFlags NON_DIFFUSE = BSDF_ALL & ~BSDF_DIFFUSE;
      if(depth + 1 < this->max_depth && bsdf->num_bxdfs(NON_DIFFUSE) > 0) {
        Vector bsdf_wi;
        float bsdf_pdf;
        BxdfFlags bsdf_flags;
        Spectrum bsdf_f = bsdf->sample_f(geom.wo, bsdf_wi, bsdf_pdf,
            NON_DIFFUSE, bsdf_flags, BsdfSample(sampler.rng));
        if(bsdf_pdf == 0.f || bsdf_f.is_black()) {
          break;
        }

        ray = Ray(geom.p, bsdf_wi, geom.ray_epsilon);
        weight *= bsdf_f * (abs_dot(bsdf_wi, geom.nn) / bsdf_pdf);
      } else {
        Spectrum photon_radiance = photon_map.estimate_radiance(
            geom.p, geom.nn, geom.wo, *bsdf, radius);
        Spectrum direct_radiance = uniform_sample_one_light(
            *this->scene, geom, *bsdf, sampler);
        assert(is_finite(photon_radiance) && is_nonnegative(photon_radiance));
        assert(is_finite(direct_radiance) && is_nonnegative(direct_radiance));
        radiance += weight * (photon_radiance + direct_radiance);
        break;
      }
    }

    return radiance;
  }

  PhotonMap SppmRenderer::compute_photon_map(CtxG&, Rng rng) const {
    uint32_t path_count = this->photon_path_count;
    std::vector<float> light_idx_samples(low_discrepancy_1d(
          1, path_count, rng));
    std::vector<Vec2> light_pos_samples(low_discrepancy_2d(
          1, path_count, rng));
    std::vector<Vec2> light_dir_samples(low_discrepancy_2d(
          1, path_count, rng));

    std::vector<Photon> photons;
    for(uint32_t path_i = 0; path_i < path_count; ++path_i) {
      uint32_t light_i = light_distrib.sample(light_idx_samples.at(path_i));
      float light_pdf = light_distrib.pdf(light_i);
      const Light& light = *this->scene->lights.at(light_i);
      LightRaySample light_sample(light_pos_samples.at(path_i),
          light_dir_samples.at(path_i));

      Ray ray;
      Normal prev_nn;
      float light_ray_pdf;
      Spectrum light_radiance = light.sample_ray_radiance(
          *this->scene, ray, prev_nn, light_ray_pdf, light_sample);
      if(light_radiance.is_black() || light_ray_pdf == 0.f) {
        continue;
      }

      Spectrum path_power = light_radiance * (abs_dot(ray.dir, prev_nn) 
        / (light_ray_pdf * light_pdf));
      for(uint32_t depth = 0; depth < this->max_photon_depth; ++depth) {
        Intersection isect;
        if(!this->scene->intersect(ray, isect)) {
          break;
        }
        auto bsdf = isect.get_bsdf();
        bool has_non_delta = bsdf->num_bxdfs(BSDF_DELTA) < bsdf->num_bxdfs();

        if(depth > 0 && has_non_delta) {
          Photon photon;
          photon.power = path_power;
          photon.p = isect.world_diff_geom.p;
          photon.wi = -ray.dir;
          photon.nn = isect.world_diff_geom.nn;
          photons.push_back(photon);
        }

        Vector bounce_wi;
        float bounce_pdf;
        BxdfFlags bounce_flags;
        Spectrum bounce_f = bsdf->sample_f(-ray.dir, bounce_wi, bounce_pdf,
            BSDF_ALL, bounce_flags, BsdfSample(rng));
        if(bounce_f.is_black() || bounce_pdf == 0.f) {
          break;
        }
        Spectrum bounce_contrib = bounce_f * (abs_dot(bounce_wi,
              isect.world_diff_geom.nn) / bounce_pdf);
        path_power *= bounce_contrib;

        if(depth > 0) {
          float survive_prob = min(0.95f, bounce_contrib.average());
          if(rng.uniform_float() > survive_prob) {
            break;
          }
          path_power /= survive_prob;
        }

        ray = Ray(isect.world_diff_geom.p, bounce_wi, isect.ray_epsilon);
      }
    }

    return PhotonMap(std::move(photons), path_count);
  }
}

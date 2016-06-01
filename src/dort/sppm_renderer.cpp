#include "dort/primitive.hpp"
#include "dort/lighting.hpp"
#include "dort/low_discrepancy.hpp"
#include "dort/sppm_renderer.hpp"

namespace dort {
  Spectrum SppmRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth, Sampler& sampler) const
  {
    Spectrum radiance(0.f);

    Intersection isect;
    if(!scene.intersect(ray, isect)) {
      for(const auto& light: scene.lights) {
        if(light->flags & LIGHT_BACKGROUND) {
          radiance += light->background_radiance(ray);
        }
      }
      return radiance;
    }

    Spectrum emitted = isect.emitted_radiance(-ray.dir);
    assert(is_finite(emitted) && is_nonnegative(emitted));
    radiance += emitted;

    LightingGeom geom;
    geom.p = isect.world_diff_geom.p;
    geom.nn = isect.world_diff_geom.nn;
    geom.wo = normalize(-ray.dir);
    geom.ray_epsilon = isect.ray_epsilon;

    std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();

    Vector bsdf_wi;
    float bsdf_pdf;
    BxdfFlags bsdf_flags;
    Spectrum bsdf_f = bsdf->sample_f(geom.wo, bsdf_wi, bsdf_pdf,
        BSDF_ALL, bsdf_flags, BsdfSample(sampler));
    if(depth < this->max_depth && bsdf_flags & (BSDF_GLOSSY | BSDF_SPECULAR)) {
      if(bsdf_pdf != 0.f && !bsdf_f.is_black()) {
        Ray recursive_ray(geom.p, bsdf_wi, geom.ray_epsilon);
        Spectrum recursive = this->get_radiance(scene,
            recursive_ray, depth + 1, sampler);
        radiance += recursive * bsdf_f * (abs_dot(bsdf_wi, geom.nn) / bsdf_pdf);
      }
    } else {
      radiance += this->photon_map.estimate_radiance(geom.p, geom.nn,
          geom.wo, *bsdf, 0.02f);
      radiance += uniform_sample_one_light(scene, geom, *bsdf, sampler);
    }

    return radiance;
  }

  void SppmRenderer::preprocess(CtxG&, const Scene& scene, Sampler& sampler) {
    DiscreteDistrib1d light_distrib(compute_light_distrib(scene));

    uint32_t path_count = this->photon_path_count;
    std::vector<float> light_idx_samples(low_discrepancy_1d(
          1, path_count, sampler.rng));
    std::vector<Vec2> light_pos_samples(low_discrepancy_2d(
          1, path_count, sampler.rng));
    std::vector<Vec2> light_dir_samples(low_discrepancy_2d(
          1, path_count, sampler.rng));

    std::vector<Photon> photons;
    for(uint32_t path_i = 0; path_i < path_count; ++path_i) {
      uint32_t light_i = light_distrib.sample(light_idx_samples.at(path_i));
      float light_pdf = light_distrib.pdf(light_i);
      const Light& light = *scene.lights.at(light_i);
      LightRaySample light_sample(light_pos_samples.at(path_i),
          light_dir_samples.at(path_i));

      Ray ray;
      Normal prev_nn;
      float light_ray_pdf;
      Spectrum light_radiance = light.sample_ray_radiance(
          scene, ray, prev_nn, light_ray_pdf, light_sample);
      if(light_radiance.is_black() || light_ray_pdf == 0.f) {
        return;
      }

      Spectrum path_power = light_radiance * (abs_dot(ray.dir, prev_nn) 
        / (light_ray_pdf * light_pdf));
      for(uint32_t depth = 0; depth < this->max_photon_depth; ++depth) {
        Intersection isect;
        if(!scene.intersect(ray, isect)) {
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
            BSDF_ALL, bounce_flags, BsdfSample(sampler));
        if(bounce_f.is_black() || bounce_pdf == 0.f) {
          break;
        }
        Spectrum bounce_contrib = bounce_f * (abs_dot(bounce_wi,
              isect.world_diff_geom.nn) / bounce_pdf);
        path_power *= bounce_contrib;

        if(depth > 0) {
          float survive_prob = min(0.95f, bounce_contrib.average());
          if(sampler.random_1d() > survive_prob) {
            break;
          }
          path_power /= survive_prob;
        }

        ray = Ray(isect.world_diff_geom.p, bounce_wi, isect.ray_epsilon);
      }
    }

    this->photon_map = PhotonMap(std::move(photons), path_count);
  }
}

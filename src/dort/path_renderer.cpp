#include "dort/camera.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/film.hpp"
#include "dort/path_renderer.hpp"
#include "dort/primitive.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  void PathRenderer::render(CtxG& ctx, Progress&) {
    this->light_distrib = compute_light_distrib(*this->scene);

    for(uint32_t i = 0; i < this->iteration_count; ++i) {
      this->iteration_tiled(ctx, [&](Vec2i pixel, Vec2& film_pos, Sampler& sampler) {
        film_pos = Vec2(pixel) + sampler.random_2d();
        return this->sample(film_pos, sampler);
      });
    }
  }

  Spectrum PathRenderer::sample(Vec2 film_pos, Sampler& sampler) const {
    Ray next_ray;
    float ray_pos_pdf;
    float ray_dir_pdf;
    Spectrum importance = this->camera->sample_ray_importance(Vec2(this->film->res),
        film_pos, next_ray, ray_pos_pdf, ray_dir_pdf, CameraSample(sampler.rng));
    float ray_pdf = ray_pos_pdf * ray_dir_pdf;
    if(ray_pdf == 0.f || importance.is_black()) { return Spectrum(0.f); }

    Spectrum radiance(0.f);
    Spectrum throughput(1.f);
    bool last_bxdf_was_specular = false;

    uint32_t bounces = 0;
    for(;;) {
      Intersection isect;
      bool isected = this->scene->intersect(next_ray, isect);

      if(bounces >= this->min_depth && (bounces == 0 || last_bxdf_was_specular)) {
        if(isected) {
          radiance += throughput * isect.eval_radiance(next_ray.orig);
        } else {
          for(const auto& light: this->scene->background_lights) {
            radiance += throughput * light->background_radiance(next_ray);
          }
        }
      }

      if(!isected || bounces >= this->max_depth) {
        break;
      }
      ++bounces;

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.p_epsilon = isect.ray_epsilon;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo_camera = normalize(-next_ray.dir);
      auto bsdf = isect.get_bsdf();

      if(bounces >= this->min_depth) {
        Spectrum direct = this->sample_direct_lighting(geom, *bsdf, sampler);
        assert(is_finite(direct) && is_nonnegative(direct));
        radiance += direct * throughput;
      }

      Vector bsdf_wi;
      float bsdf_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_light_f(geom.wo_camera,
          this->only_direct ? BSDF_ALL_HEMISPHERES | BSDF_DELTA : BSDF_ALL,
          bsdf_wi, bsdf_pdf, bsdf_flags, BsdfSample(sampler.rng));

      if(bsdf_f.is_black() || bsdf_pdf == 0.f) { break; }
      throughput = throughput * bsdf_f * (abs_dot(bsdf_wi, geom.nn) / bsdf_pdf);
      assert(is_finite(throughput) && is_nonnegative(throughput));

      next_ray = Ray(geom.p, bsdf_wi, geom.p_epsilon);
      last_bxdf_was_specular = bsdf_flags & BSDF_DELTA;

      if(bounces > this->max_depth / 2 && bounces > this->min_depth) {
        float term_prob = max(0.05f, 1.f - throughput.average());
        if(sampler.random_1d() < term_prob) {
          break;
        }
        throughput = throughput / term_prob;
      }
    }

    return radiance * importance / ray_pdf;
  }

  Spectrum PathRenderer::sample_direct_lighting(const LightingGeom& geom,
      const Bsdf& bsdf, Sampler& sampler) const
  {
    if(this->sample_all_lights) {
      Spectrum radiance_sum(0.f);
      for(const auto& light: this->scene->lights) {
        radiance_sum += this->estimate_direct(geom, *light, bsdf, sampler);
      }
      return radiance_sum;
    } else {
      uint32_t light_i = this->light_distrib.sample(sampler.random_1d());
      float light_pick_pdf = this->light_distrib.pdf(light_i);
      if(light_pick_pdf == 0.f) { return Spectrum(0.f); }
      const Light& light = *this->scene->lights.at(light_i);
      return this->estimate_direct(geom, light, bsdf, sampler) / light_pick_pdf;
    }
  }

  Spectrum PathRenderer::estimate_direct(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler) const
  {
    bool use_bsdf = this->direct_strategy != DirectStrategy::SAMPLE_LIGHT
      && !(light.flags & LIGHT_DELTA) && (light.flags & (LIGHT_AREA | LIGHT_BACKGROUND));
    bool use_light = this->direct_strategy != DirectStrategy::SAMPLE_BSDF
      && bsdf.num_bxdfs(BSDF_DELTA) < bsdf.num_bxdfs();

    Spectrum bsdf_contrib = use_bsdf
      ? this->estimate_direct_bsdf(geom, light, bsdf, sampler, use_light)
      : Spectrum(0.f);
    Spectrum light_contrib = use_light
      ? this->estimate_direct_light(geom, light, bsdf, sampler, use_bsdf)
      : Spectrum(0.f);
    return bsdf_contrib + light_contrib;
  }

  Spectrum PathRenderer::estimate_direct_bsdf(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const
  {
    Vector wi_light;
    float wi_dir_pdf;
    BxdfFlags bsdf_flags;
    Spectrum bsdf_f = bsdf.sample_light_f(geom.wo_camera, BSDF_ALL, wi_light,
        wi_dir_pdf, bsdf_flags, BsdfSample(sampler.rng));
    if(wi_dir_pdf == 0.f || bsdf_f.is_black()) { return Spectrum(0.f); }

    Ray light_ray(geom.p, wi_light, geom.p_epsilon);
    Intersection light_isect;
    bool isected = this->scene->intersect(light_ray, light_isect);

    Spectrum radiance(0.f);
    if(isected && (light.flags & LIGHT_AREA)) {
      if(light_isect.get_area_light() == &light) {
        radiance = light.eval_radiance(light_isect.world_diff_geom.p,
            light_isect.world_diff_geom.nn, geom.p);
      }
    } else if(!isected && (light.flags & LIGHT_BACKGROUND)) {
      radiance = light.background_radiance(light_ray);
    }
    if(radiance.is_black()) { return Spectrum(0.f); }

    float weight = 1.f;
    if(use_mis) {
      float wi_light_dir_pdf = light.pivot_radiance_pdf(wi_light, geom.p);
      weight = wi_dir_pdf / (wi_dir_pdf + wi_light_dir_pdf);
    }

    return bsdf_f * radiance * (weight * abs_dot(geom.nn, wi_light) / wi_dir_pdf);
  }

  Spectrum PathRenderer::estimate_direct_light(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const
  {
    Vector wi_light;
    float wi_dir_pdf;
    ShadowTest shadow;
    Spectrum radiance = light.sample_pivot_radiance(geom.p, geom.p_epsilon,
        wi_light, wi_dir_pdf, shadow, LightSample(sampler.rng));
    if(wi_dir_pdf == 0.f || radiance.is_black()) { return Spectrum(0.f); }

    Spectrum bsdf_f = bsdf.eval_f(wi_light, geom.wo_camera, BSDF_ALL);
    if(bsdf_f.is_black()) { return Spectrum(0.f); }
    if(!shadow.visible(*this->scene)) { return Spectrum(0.f); }

    float weight = 1.f;
    if(use_mis) {
      float wi_bsdf_dir_pdf = bsdf.light_f_pdf(wi_light, geom.wo_camera, BSDF_ALL);
      weight = wi_dir_pdf / (wi_dir_pdf + wi_bsdf_dir_pdf);
    }

    return bsdf_f * radiance * (weight * abs_dot(geom.nn, wi_light) / wi_dir_pdf);
  }
}

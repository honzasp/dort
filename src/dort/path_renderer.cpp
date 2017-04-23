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
        film_pos = Vec2(pixel) + (i == 0 ? Vec2(0.5f, 0.5f) : sampler.random_2d());
        return this->sample(film_pos, sampler);
      });
    }
  }

  Spectrum PathRenderer::sample(Vec2 film_pos, Sampler& sampler) const {
    // sample the initial camera ray
    Ray next_ray;
    float ray_pos_pdf;
    float ray_dir_pdf;
    Spectrum importance = this->camera->sample_ray_importance(Vec2(this->film->res),
        film_pos, next_ray, ray_pos_pdf, ray_dir_pdf, CameraSample(sampler.rng));
    float ray_pdf = ray_pos_pdf * ray_dir_pdf;
    if(ray_pdf == 0.f || importance.is_black()) { return Spectrum(0.f); }

    Spectrum radiance_sum(0.f);
    Spectrum throughput(1.f);

    uint32_t bounces = 0;
    bool last_bounce_was_delta = false;
    for(;;) {
      Intersection isect;
      bool isected = this->scene->intersect(next_ray, isect);

      if((bounces == 0 || last_bounce_was_delta)
          && bounces >= this->min_depth && bounces <= this->max_depth) 
      {
        // add radiance sampled directly from this vertex, but only if it is the
        // first bounce (this is the only opportunity to add radiance from
        // lights directly visible to the camera) or if the last bounce was
        // delta (we don't sample delta components from BSDFs in the direct
        // lighting computation).
        if(isected) {
          radiance_sum += throughput * isect.eval_radiance(next_ray.orig);
        } else {
          for(const auto& light: this->scene->background_lights) {
            radiance_sum += throughput * light->background_radiance(next_ray);
          }
        }
      }
      bounces += 1;
      if(!isected || bounces > this->max_depth) { break; }

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.p_epsilon = isect.ray_epsilon;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo_camera = normalize(-next_ray.dir);
      auto bsdf = isect.get_bsdf();

      if(bounces >= this->min_depth) {
        // add the direct lighting radiance. only non-delta components of the
        // BSDF are evaluated (direct lighting through delta components is
        // handled by adding the radiance emitted from sampled vertex if the
        // previous bounce was delta)
        Spectrum radiance = this->sample_direct_lighting(geom, *bsdf, sampler);
        assert(is_finite(radiance) && is_nonnegative(radiance));
        radiance_sum += radiance * throughput;
      }

      // sample the next bounce from BSDF
      Vector bsdf_wi;
      float bsdf_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_light_f(geom.wo_camera,
          // 1. if we gather only direct lighting, we still do specular bounces
          // 2. if this is the last bounce, the only remaining contribution
          //    could be emitted light if the bounce was delta, so there is no
          //    point in sampling non-delta components
          (this->only_direct || bounces == this->max_depth) 
            ? BSDF_MODES | BSDF_DELTA : BSDF_ALL,
          bsdf_wi, bsdf_pdf, bsdf_flags, BsdfSample(sampler.rng));
      if(bsdf_f.is_black() || bsdf_pdf == 0.f) { break; }

      Spectrum bounce_contrib = bsdf_f * (abs_dot(bsdf_wi, geom.nn) / bsdf_pdf);
      last_bounce_was_delta = bsdf_flags & BSDF_DELTA;
      assert(is_finite(bounce_contrib) && is_nonnegative(bounce_contrib));
      throughput = throughput * bounce_contrib;

      if(bounces == this->max_depth && !last_bounce_was_delta) {
        // terminate the path early if the last vertex can have no contribution
        // (saves one ray trace)
        break; 
      } else if(bounces >= 2 && bounces > this->min_depth) {
        // Russian roulette termination
        float survive_prob = clamp(bounce_contrib.average(), 0.1f, 0.99f);
        if(sampler.random_1d() > survive_prob) { break; }
        throughput = throughput / survive_prob;
      }
      next_ray = Ray(geom.p, bsdf_wi, geom.p_epsilon);
    }

    return radiance_sum * importance / ray_pdf;
  }

  Spectrum PathRenderer::sample_direct_lighting(const LightingGeom& geom,
      const Bsdf& bsdf, Sampler& sampler) const
  {
    if(bsdf.bxdf_count(BSDF_ALL & (~BSDF_DELTA)) == 0) { return Spectrum(0.f); }

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
      Spectrum radiance = this->estimate_direct(geom, light, bsdf, sampler);
      return radiance / light_pick_pdf;
    }
  }


  Spectrum PathRenderer::estimate_direct(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler) const
  {
    bool use_bsdf = this->direct_strategy != DirectStrategy::SAMPLE_LIGHT
      && !(light.flags & LIGHT_DELTA) && (light.flags & (LIGHT_AREA | LIGHT_BACKGROUND));
    bool use_light = this->direct_strategy != DirectStrategy::SAMPLE_BSDF;

    Spectrum bsdf_contrib = !use_bsdf ? Spectrum(0.f)
      : this->estimate_direct_from_bsdf(geom, light, bsdf, sampler, use_light);
    Spectrum light_contrib = !use_light ? Spectrum(0.f)
      : this->estimate_direct_from_light(geom, light, bsdf, sampler, use_bsdf);
    return bsdf_contrib + light_contrib;
  }

  Spectrum PathRenderer::estimate_direct_from_bsdf(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const
  {
    Vector wi_light;
    float wi_dir_pdf;
    BxdfFlags bsdf_flags;
    Spectrum bsdf_f = bsdf.sample_light_f(geom.wo_camera, BSDF_ALL & (~BSDF_DELTA),
        wi_light, wi_dir_pdf, bsdf_flags, BsdfSample(sampler.rng));
    if(wi_dir_pdf == 0.f || bsdf_f.is_black()) { return Spectrum(0.f); }
    assert(!(bsdf_flags & BSDF_DELTA));

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

  Spectrum PathRenderer::estimate_direct_from_light(const LightingGeom& geom,
      const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const
  {
    Vector wi_light;
    float wi_dir_pdf;
    ShadowTest shadow;
    Spectrum radiance = light.sample_pivot_radiance(geom.p, geom.p_epsilon,
        wi_light, wi_dir_pdf, shadow, LightSample(sampler.rng));
    if(wi_dir_pdf == 0.f || radiance.is_black()) { return Spectrum(0.f); }

    Spectrum bsdf_f = bsdf.eval_f(wi_light, geom.wo_camera, BSDF_ALL & (~BSDF_DELTA));
    if(bsdf_f.is_black()) { return Spectrum(0.f); }
    if(!shadow.visible(*this->scene)) { return Spectrum(0.f); }

    float weight = 1.f;
    if(use_mis) {
      float wi_bsdf_dir_pdf = bsdf.light_f_pdf(wi_light,
          geom.wo_camera, BSDF_ALL & (~BSDF_DELTA));
      weight = wi_dir_pdf / (wi_dir_pdf + wi_bsdf_dir_pdf);
    }

    return bsdf_f * radiance * (weight * abs_dot(geom.nn, wi_light) / wi_dir_pdf);
  }
}

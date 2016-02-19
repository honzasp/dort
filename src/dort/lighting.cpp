#include "dort/lighting.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/primitive.hpp"
#include "dort/renderer.hpp"
#include "dort/stats.hpp"

namespace dort {
  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      slice<const LightSamplesIdxs> light_samples_idxs,
      slice<const BsdfSamplesIdxs> bsdf_samples_idxs)
  {
    stat_count(COUNTER_UNIFORM_SAMPLE_ALL_LIGHTS);
    StatTimer t(TIMER_UNIFORM_SAMPLE_ALL_LIGHTS);
    assert(light_samples_idxs.size() == scene.lights.size());
    assert(bsdf_samples_idxs.size() == scene.lights.size());

    Spectrum radiance(0.f);
    for(uint32_t l = 0; l < scene.lights.size(); ++l) {
      const auto& light_idxs = light_samples_idxs.at(l);
      const auto& bsdf_idxs = bsdf_samples_idxs.at(l);
      assert(light_idxs.count == bsdf_idxs.count);

      Spectrum light_radiance(0.f);
      for(uint32_t i = 0; i < light_idxs.count; ++i) {
        light_radiance += estimate_direct(scene, geom,
            bsdf, *scene.lights.at(l), BxdfFlags(BSDF_ALL & ~(BSDF_SPECULAR)),
            LightSample(sampler, light_idxs, i),
            BsdfSample(sampler, bsdf_idxs, i));
      }
      assert(is_finite(light_radiance));
      radiance += light_radiance / float(light_idxs.count);
    }

    return radiance;
  }

  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler)
  {
    stat_count(COUNTER_UNIFORM_SAMPLE_ALL_LIGHTS);
    StatTimer t(TIMER_UNIFORM_SAMPLE_ALL_LIGHTS);
    Spectrum radiance(0.f);
    for(const auto& light: scene.lights) {
      uint32_t num_samples = max(1u, light->num_samples);
      Spectrum light_radiance(0.f);
      for(uint32_t i = 0; i < num_samples; ++i) {
        light_radiance += estimate_direct(scene, geom,
            bsdf, *light, BxdfFlags(BSDF_ALL & ~(BSDF_SPECULAR)),
            LightSample(sampler), BsdfSample(sampler));
      }
      assert(is_finite(light_radiance));
      radiance += light_radiance / float(num_samples);
    }
    return radiance;
  }

  Spectrum estimate_direct(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf,
      const Light& light, BxdfFlags bxdf_flags,
      LightSample light_sample, BsdfSample bsdf_sample)
  {
    stat_count(COUNTER_ESTIMATE_DIRECT);
    StatTimer t(TIMER_ESTIMATE_DIRECT);
    Spectrum light_contrib(0.f);
    Spectrum bsdf_contrib(0.f);

    {
      // sample from light
      Vector wi;
      float light_pdf;
      ShadowTest shadow;
      Spectrum light_radiance = light.sample_radiance(geom.p,
        geom.ray_epsilon, wi, light_pdf, shadow, light_sample);
      if(!light_radiance.is_black() && light_pdf > 0.f) {
        Spectrum bsdf_f = bsdf.f(geom.wo, wi, bxdf_flags);
        if(!bsdf_f.is_black() && shadow.visible(scene)) {
          float weight = 1.f;
          if(!light.is_delta()) {
            float bsdf_pdf = bsdf.f_pdf(geom.wo, wi, bxdf_flags);
            weight = mis_power_heuristic(1, light_pdf, 1, bsdf_pdf);
          }
          light_contrib = bsdf_f * light_radiance * (abs_dot(wi, geom.nn) 
            * weight / light_pdf);
          assert(is_finite(light_contrib));
        }
      }
    }

    if(!light.is_delta()) {
      // sample from BSDF
      Vector wi;
      float bsdf_pdf;
      BxdfFlags sampled_flags;
      Spectrum bsdf_f = bsdf.sample_f(geom.wo, wi, bsdf_pdf, bxdf_flags,
          sampled_flags, bsdf_sample);
      if(!bsdf_f.is_black() && bsdf_pdf > 0.f) {
        float weight = 1.f;
        if(!(sampled_flags & BSDF_SPECULAR)) {
          float light_pdf = light.radiance_pdf(geom.p, wi);
          weight = mis_power_heuristic(1, bsdf_pdf, 1, light_pdf);
        }

        Spectrum light_radiance(0.f);
        Ray light_ray(geom.p, wi, geom.ray_epsilon, INFINITY);
        Intersection light_isect;
        if(scene.intersect(light_ray, light_isect)) {
          const AreaLight* area_light =
            light_isect.primitive->get_area_light(light_isect.world_diff_geom);
          if(area_light == &light) {
            light_radiance = area_light->emitted_radiance(
                light_isect.world_diff_geom.p, light_isect.world_diff_geom.nn, -wi);
          }
        } else {
          light_radiance = light.background_radiance(light_ray);
        }

        bsdf_contrib = bsdf_f * light_radiance * (abs_dot(wi, geom.nn)
          * weight / bsdf_pdf);
        assert(is_finite(bsdf_contrib));
      }
    }

    return light_contrib + bsdf_contrib;
  }

  Spectrum trace_specular(const Renderer& renderer,
      const Scene& scene, const LightingGeom& geom, const Bsdf& bsdf,
      BxdfFlags flags, uint32_t depth, Sampler& sampler)
  {
    stat_count(COUNTER_TRACE_SPECULAR);
    StatTimer t(TIMER_TRACE_SPECULAR);

    Vector wi;
    float pdf;
    BxdfFlags sampled_flags;
    Spectrum bsdf_f = bsdf.sample_f(geom.wo, wi, pdf,
        BxdfFlags(BSDF_SPECULAR | flags), sampled_flags,
        BsdfSample(sampler));
    if(!bsdf_f.is_black() && pdf > 0) {
      Ray spec_ray(geom.p, wi, geom.ray_epsilon, INFINITY);
      Spectrum radiance = renderer.get_radiance(scene, spec_ray, depth + 1, sampler);
      assert(is_finite(bsdf_f));
      return radiance * bsdf_f * (abs_dot(wi, geom.nn) / pdf);
    } else {
      return Spectrum(0.f);
    }
  }
}
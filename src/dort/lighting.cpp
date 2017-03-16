#include "dort/lighting.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/primitive.hpp"
#include "dort/sample_renderer.hpp"
#include "dort/stats.hpp"

namespace dort {
  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      slice<const LightSamplesIdxs> light_samples_idxs,
      slice<const BsdfSamplesIdxs> bsdf_samples_idxs,
      DirectStrategy strategy)
  {
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
            bsdf, *scene.lights.at(l), BSDF_ALL & ~BSDF_DELTA,
            LightSample(sampler, light_idxs, i),
            BsdfSample(sampler, bsdf_idxs, i), strategy);
      }
      assert(is_finite(light_radiance));
      radiance += light_radiance / float(light_idxs.count);
    }

    return radiance;
  }

  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      DirectStrategy strategy)
  {
    StatTimer t(TIMER_UNIFORM_SAMPLE_ALL_LIGHTS);
    Spectrum radiance(0.f);
    for(const auto& light: scene.lights) {
      uint32_t num_samples = max(1u, light->num_samples);
      Spectrum light_radiance(0.f);
      for(uint32_t i = 0; i < num_samples; ++i) {
        light_radiance += estimate_direct(scene, geom,
            bsdf, *light, BSDF_ALL & ~BSDF_DELTA,
            LightSample(sampler.rng), BsdfSample(sampler.rng), strategy);
      }
      assert(is_finite(light_radiance));
      radiance += light_radiance / float(num_samples);
    }
    return radiance;
  }

  Spectrum uniform_sample_one_light(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler&,
      float u_select, const LightSample& light_sample,
      const BsdfSample& bsdf_sample, DirectStrategy strategy)
  {
    float num_lights = float(scene.lights.size());
    uint32_t light_idx = clamp(floor_int32(num_lights * u_select),
        0, int32_t(scene.lights.size()) - 1);
    return num_lights * estimate_direct(scene, geom, bsdf,
        *scene.lights.at(light_idx), BSDF_ALL & ~BSDF_DELTA,
        light_sample, bsdf_sample, strategy);
  }

  Spectrum uniform_sample_one_light(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      DirectStrategy strategy)
  {
    return uniform_sample_one_light(scene, geom, bsdf, sampler,
        sampler.random_1d(), LightSample(sampler.rng),
        BsdfSample(sampler.rng), strategy);
  }

  Spectrum estimate_direct(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf,
      const Light& light, BxdfFlags bxdf_flags,
      LightSample light_sample, BsdfSample bsdf_sample,
      DirectStrategy strategy)
  {
    StatTimer t(TIMER_ESTIMATE_DIRECT);
    Spectrum light_contrib(0.f);
    Spectrum bsdf_contrib(0.f);

    bool use_light = strategy != DirectStrategy::SAMPLE_BSDF;
    bool use_bsdf = strategy != DirectStrategy::SAMPLE_LIGHT;
    bool use_mis = strategy == DirectStrategy::MIS;

    if(use_light) {
      Vector wi;
      float wi_light_pdf;
      ShadowTest shadow;
      Spectrum light_radiance = light.sample_pivot_radiance(geom.p,
        geom.ray_epsilon, wi, wi_light_pdf, shadow, light_sample);
      if(!light_radiance.is_black() && wi_light_pdf > 0.f) {
        Spectrum bsdf_f = bsdf.eval_f(geom.wo_camera, wi, bxdf_flags);
        if(!bsdf_f.is_black() && (shadow.visible(scene))) {
          float weight = 1.f;
          if(use_mis && !(light.flags & LIGHT_DELTA)) {
            float wi_bsdf_pdf = bsdf.light_f_pdf(wi, geom.wo_camera, bxdf_flags);
            weight = mis_power_heuristic(1, wi_light_pdf, 1, wi_bsdf_pdf);
          }
          light_contrib = bsdf_f * light_radiance * (abs_dot(wi, geom.nn) 
            * weight / wi_light_pdf);
          assert(is_finite(light_contrib) && is_nonnegative(bsdf_contrib));
        }
      }
    }

    if(use_bsdf && !(light.flags & LIGHT_DELTA)) {
      Vector wi;
      float wi_bsdf_pdf;
      BxdfFlags sampled_flags;
      Spectrum bsdf_f = bsdf.sample_light_f(geom.wo_camera, bxdf_flags,
          wi, wi_bsdf_pdf, sampled_flags, bsdf_sample);
      if(!bsdf_f.is_black() && wi_bsdf_pdf > 0.f) {
        float weight = 1.f;
        if(use_mis && !(sampled_flags & BSDF_DELTA)) {
          float wi_light_pdf = light.pivot_radiance_pdf(geom.p, wi);
          weight = mis_power_heuristic(1, wi_bsdf_pdf, 1, wi_light_pdf);
        }

        Spectrum light_radiance(0.f);
        Ray light_ray(geom.p, wi, geom.ray_epsilon, INFINITY);

        if(light.flags & LIGHT_AREA) {
          Intersection light_isect;
          if(scene.intersect(light_ray, light_isect)) {
            const Light* area_light =
              light_isect.primitive->get_area_light(light_isect.world_diff_geom);
            if(area_light == &light) {
              light_radiance = area_light->eval_radiance(
                  light_isect.world_diff_geom.p, light_isect.world_diff_geom.nn,
                  geom.p);
            }
          }
        } else if(light.flags & LIGHT_BACKGROUND) {
          if(!scene.intersect_p(light_ray)) {
            light_radiance = light.background_radiance(light_ray);
          }
        }

        bsdf_contrib = bsdf_f * light_radiance * (abs_dot(wi, geom.nn)
          * weight / wi_bsdf_pdf);
        assert(is_finite(bsdf_contrib) && is_nonnegative(bsdf_contrib));
      }
    }

    return light_contrib + bsdf_contrib;
  }

  Spectrum trace_specular(const SampleRenderer& renderer, Vec2 film_pos,
      const Scene& scene, const LightingGeom& geom, const Bsdf& bsdf,
      BxdfFlags flags, uint32_t depth, Sampler& sampler)
  {
    StatTimer t(TIMER_TRACE_SPECULAR);

    Vector wi;
    float wi_pdf;
    BxdfFlags sampled_flags;
    Spectrum bsdf_f = bsdf.sample_light_f(geom.wo_camera, BSDF_DELTA | flags,  
        wi, wi_pdf, sampled_flags, BsdfSample(sampler.rng));
    assert(is_finite(bsdf_f) && is_nonnegative(bsdf_f));
    if(!bsdf_f.is_black() && wi_pdf != 0.f) {
      Ray spec_ray(geom.p, wi, geom.ray_epsilon, INFINITY);
      Spectrum radiance = renderer.get_radiance(scene, spec_ray, film_pos,
          depth + 1, sampler);
      return radiance * bsdf_f * (abs_dot(wi, geom.nn) / wi_pdf);
    } else {
      return Spectrum(0.f);
    }
  }

  DiscreteDistrib1d compute_light_distrib(const Scene& scene) {
    std::vector<float> powers(scene.lights.size());
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      const auto& light = scene.lights.at(i);
      float power = light->approximate_power(scene).average();
      float weight = float(light->num_samples);
      powers.at(i) = power * weight;
    }
    return DiscreteDistrib1d(powers);
  }
}

#include "dort/light.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/primitive.hpp"
#include "dort/direct_renderer.hpp"

namespace dort {
  Spectrum DirectRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth, Rng& rng) const
  {
    Spectrum radiance(0.f);

    Intersection isect;
    if(!scene.primitive->intersect(ray, isect)) {
      for(const auto& light: scene.lights) {
        radiance = radiance + light->background_radiance(ray);
      }
      return radiance;
    }

    const AreaLight* area_light = isect.primitive->get_area_light(isect.diff_geom);
    if(area_light) {
      Spectrum emitted = area_light->emitted_radiance(
          isect.diff_geom.p, isect.diff_geom.nn, -ray.dir);
      assert(is_finite(emitted));
      radiance = radiance + emitted;
    }

    std::unique_ptr<Bsdf> bsdf = isect.primitive->get_bsdf(isect.diff_geom);
    radiance = radiance + uniform_sample_all_lights(scene, ray, isect, *bsdf, rng);

    if(depth < this->max_depth) {
      Spectrum reflection = trace_specular(scene, *this, ray,
          isect, *bsdf, BSDF_REFLECTION, depth, rng);
      Spectrum refraction = trace_specular(scene, *this, ray,
          isect, *bsdf, BSDF_TRANSMISSION, depth, rng);
      assert(is_finite(reflection));
      assert(is_finite(refraction));
      radiance = radiance + reflection + refraction;
    }

    return radiance;
  }

  Spectrum DirectRenderer::uniform_sample_all_lights(const Scene& scene,
      const Ray& ray, const Intersection& isect, const Bsdf& bsdf,
      Rng& rng)
  {
    Spectrum radiance(0.f);

    for(const auto& light: scene.lights) {
      Spectrum light_radiance(0.f);
      uint32_t num_samples = max(1u, light->num_samples);
      for(uint32_t i = 0; i < num_samples; ++i) {
        light_radiance = light_radiance + estimate_direct(scene, ray, isect, 
            bsdf, *light, BxdfFlags(BSDF_ALL & ~(BSDF_SPECULAR)), rng);
      }
      assert(is_finite(light_radiance));
      radiance = radiance + light_radiance / float(num_samples);
    }

    return radiance;
  }

  Spectrum DirectRenderer::estimate_direct(const Scene& scene,
      const Ray& ray, const Intersection& isect, const Bsdf& bsdf,
      const Light& light, BxdfFlags bxdf_flags, Rng& rng)
  {
    Spectrum light_contrib(0.f);
    Spectrum bsdf_contrib(0.f);

    Vector wo = normalize(-ray.dir);
    Normal n = isect.diff_geom.nn;

    {
      // sample from light
      Vector wi;
      float light_pdf;
      ShadowTest shadow;
      Spectrum light_radiance = light.sample_radiance(isect.diff_geom.p,
        isect.ray_epsilon, wi, light_pdf, shadow, rng);
      if(!light_radiance.is_black() && light_pdf > 0.f) {
        Spectrum bsdf_f = bsdf.f(wo, wi, bxdf_flags);
        if(!bsdf_f.is_black() && shadow.visible(scene)) {
          float weight = 1.f;
          if(!light.is_delta()) {
            float bsdf_pdf = bsdf.f_pdf(wo, wi, bxdf_flags);
            weight = mis_power_heuristic(1, light_pdf, 1, bsdf_pdf);
          }
          light_contrib = bsdf_f * light_radiance * abs_dot(wi, n) 
            * (weight / light_pdf);
          assert(is_finite(light_contrib));
        }
      }
    }

    if(!light.is_delta()) {
      // sample from BSDF
      Vector wi;
      float bsdf_pdf;
      BxdfFlags sampled_flags;
      Spectrum bsdf_f = bsdf.sample_f(wo, wi, bsdf_pdf, bxdf_flags,
          sampled_flags, rng);
      if(!bsdf_f.is_black() && bsdf_pdf > 0.f) {
        float weight = 1.f;
        if(!(sampled_flags & BSDF_SPECULAR)) {
          float light_pdf = light.radiance_pdf(isect.diff_geom.p, wi);
          weight = mis_power_heuristic(1, bsdf_pdf, 1, light_pdf);
        }

        Spectrum light_radiance(0.f);
        Ray light_ray(isect.diff_geom.p, wi, isect.ray_epsilon, INFINITY);
        Intersection light_isect;
        if(scene.primitive->intersect(light_ray, light_isect)) {
          const AreaLight* area_light =
            light_isect.primitive->get_area_light(light_isect.diff_geom);
          if(area_light == &light) {
            light_radiance = area_light->emitted_radiance(
                light_isect.diff_geom.p, light_isect.diff_geom.nn, -wi);
          }
        } else {
          light_radiance = light.background_radiance(light_ray);
        }

        bsdf_contrib = bsdf_f * light_radiance * abs_dot(wi, n)
          * (weight / bsdf_pdf);
        assert(is_finite(bsdf_contrib));
      }
    }

    return light_contrib + bsdf_contrib;
  }

  Spectrum DirectRenderer::trace_specular(const Scene& scene,
      const Renderer& renderer, const Ray& ray, const Intersection& isect,
      const Bsdf& bsdf, BxdfFlags flags, uint32_t depth, Rng& rng)
  {
    Normal n = isect.diff_geom.nn;
    Vector wo = -ray.dir;
    Vector wi;
    float pdf;
    BxdfFlags sampled_flags;
    Spectrum bsdf_f = bsdf.sample_f(wo, wi, pdf,
        BxdfFlags(BSDF_SPECULAR | flags), sampled_flags, rng);
    if(!bsdf_f.is_black() && pdf > 0) {
      Ray ray(isect.diff_geom.p, wi, isect.ray_epsilon, INFINITY);
      Spectrum radiance = renderer.get_radiance(scene, ray, depth + 1, rng);
      assert(is_finite(bsdf_f));
      return bsdf_f * radiance * abs_dot(wi, n) / pdf;
    } else {
      return Spectrum(0.f);
    }
  }
}

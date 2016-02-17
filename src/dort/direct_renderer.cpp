#include "dort/light.hpp"
#include "dort/lighting.hpp"
#include "dort/primitive.hpp"
#include "dort/direct_renderer.hpp"

namespace dort {
  Spectrum DirectRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth, Sampler& sampler) const 
  {
    Spectrum radiance(0.f);

    Intersection isect;
    if(!scene.primitive->intersect(ray, isect)) {
      for(const auto& light: scene.lights) {
        radiance += light->background_radiance(ray);
      }
      return radiance;
    }

    const AreaLight* area_light = isect.primitive->get_area_light(isect.frame_diff_geom);
    if(area_light) {
      Spectrum emitted = area_light->emitted_radiance(
          isect.world_diff_geom.p, isect.world_diff_geom.nn, -ray.dir);
      assert(is_finite(emitted));
      radiance += emitted;
    }

    LightingGeom geom;
    geom.p = isect.world_diff_geom.p;
    geom.nn = isect.world_diff_geom.nn;
    geom.wo = normalize(-ray.dir);
    geom.ray_epsilon = isect.ray_epsilon;

    std::unique_ptr<Bsdf> bsdf = isect.primitive->get_bsdf(isect.frame_diff_geom);
    radiance += uniform_sample_all_lights(scene, geom, *bsdf, sampler,
        make_slice(this->light_samples_idxs),
        make_slice(this->bsdf_samples_idxs));

    if(depth < this->max_depth) {
      Spectrum reflection = trace_specular(*this, scene,
          geom, *bsdf, BSDF_REFLECTION, depth, sampler);
      Spectrum refraction = trace_specular(*this, scene,
          geom, *bsdf, BSDF_TRANSMISSION, depth, sampler);
      assert(is_finite(reflection));
      assert(is_finite(refraction));
      radiance += reflection + refraction;
    }

    return radiance;
  }

  void DirectRenderer::preprocess_(const Scene& scene, Sampler& sampler) {
    for(const auto& light: scene.lights) {
      uint32_t num_samples = max(1u, sampler.round_count(light->num_samples));
      this->light_samples_idxs.push_back(LightSample::request(sampler, num_samples));
      this->bsdf_samples_idxs.push_back(BsdfSample::request(sampler, num_samples));
    }
  }
}

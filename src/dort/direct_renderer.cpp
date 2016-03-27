#include "dort/direct_renderer.hpp"
#include "dort/light.hpp"
#include "dort/lighting.hpp"
#include "dort/primitive.hpp"
#include "dort/stats.hpp"

namespace dort {
  Spectrum DirectRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth, Sampler& sampler) const 
  {
    StatTimer t(TIMER_DIRECT_GET_RADIANCE);
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
    radiance += uniform_sample_all_lights(scene, geom, *bsdf, sampler,
        make_slice(this->light_samples_idxs),
        make_slice(this->bsdf_samples_idxs));
    stat_sample_int(DISTRIB_INT_BSDF_NUM_BXDFS, bsdf->num_bxdfs());

    if(depth < this->max_depth) {
      Spectrum reflection = trace_specular(*this, scene,
          geom, *bsdf, BSDF_REFLECTION, depth, sampler);
      Spectrum refraction = trace_specular(*this, scene,
          geom, *bsdf, BSDF_TRANSMISSION, depth, sampler);
      assert(is_finite(reflection) && is_nonnegative(reflection));
      assert(is_finite(refraction) && is_nonnegative(reflection));
      radiance += reflection + refraction;
    }

    return radiance;
  }

  void DirectRenderer::do_preprocess(const Scene& scene, Sampler& sampler) {
    for(const auto& light: scene.lights) {
      uint32_t num_samples = max(1u, sampler.round_count(light->num_samples));
      this->light_samples_idxs.push_back(LightSample::request(sampler, num_samples));
      this->bsdf_samples_idxs.push_back(BsdfSample::request(sampler, num_samples));
    }
  }
}

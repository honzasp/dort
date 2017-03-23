#pragma once
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/geometry.hpp"
#include "dort/light.hpp"

namespace dort {
  struct LightingGeom {
    Point p;
    Normal nn;
    Vector wo_camera;
    float ray_epsilon;
  };

  enum class DirectStrategy {
    SAMPLE_BSDF,
    SAMPLE_LIGHT,
    MIS,
  };

  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      slice<const LightSamplesIdxs> light_samples_idxs,
      slice<const BsdfSamplesIdxs> bsdf_samples_idxs,
      DirectStrategy strategy = DirectStrategy::MIS);
  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      DirectStrategy strategy = DirectStrategy::MIS);

  Spectrum uniform_sample_one_light(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      float u_select, const LightSample& light_sample,
      const BsdfSample& bsdf_sample,
      DirectStrategy strategy = DirectStrategy::MIS);
  Spectrum uniform_sample_one_light(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      DirectStrategy strategy = DirectStrategy::MIS);

  Spectrum estimate_direct(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf,
      const Light& light, BxdfFlags bxdf_flags,
      LightSample light_sample, BsdfSample bsdf_sample,
      DirectStrategy strategy);
  Spectrum trace_specular(const SampleRenderer& renderer, Vec2 film_pos,
      const Scene& scene, const LightingGeom& geom, const Bsdf& bsdf,
      BxdfFlags flags, uint32_t depth, Sampler& sampler);

  DiscreteDistrib1d compute_light_distrib(const Scene& scene,
      bool only_background = false);
}

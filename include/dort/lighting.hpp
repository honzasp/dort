#pragma once
#include "dort/bsdf.hpp"
#include "dort/geometry.hpp"
#include "dort/light.hpp"

namespace dort {
  struct LightingGeom {
    Point p;
    Normal nn;
    Vector wo;
    float ray_epsilon;
  };

  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
      slice<const LightSamplesIdxs> light_samples_idxs,
      slice<const BsdfSamplesIdxs> bsdf_samples_idxs);
  Spectrum uniform_sample_all_lights(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler);
  Spectrum estimate_direct(const Scene& scene,
      const LightingGeom& geom, const Bsdf& bsdf,
      const Light& light, BxdfFlags bxdf_flags,
      LightSample light_sample, BsdfSample bsdf_sample);
  Spectrum trace_specular(const Renderer& renderer,
      const Scene& scene, const LightingGeom& geom, const Bsdf& bsdf,
      BxdfFlags flags, uint32_t depth, Sampler& sampler);
}

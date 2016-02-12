#pragma once
#include "dort/bsdf.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class DirectRenderer final: public Renderer {
    uint32_t max_depth;
  public:
    DirectRenderer(uint32_t max_depth):
      max_depth(max_depth) {}
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Rng& rng) const override final;

    struct LightingGeom {
      Point p;
      Normal nn;
      Vector wo;
      float ray_epsilon;
    };

    static Spectrum uniform_sample_all_lights(const Scene& scene,
        const LightingGeom& geom, const Bsdf& bsdf, Rng& rng);
    static Spectrum estimate_direct(const Scene& scene,
        const LightingGeom& geom, const Bsdf& bsdf,
        const Light& light, BxdfFlags bxdf_flags, Rng& rng);
    static Spectrum trace_specular(const Scene& scene,
        const Renderer& renderer, const LightingGeom& geom,
        const Bsdf& bsdf, BxdfFlags flags, uint32_t depth, Rng& rng);
  };
}

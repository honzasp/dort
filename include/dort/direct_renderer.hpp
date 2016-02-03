#pragma once
#include "dort/bsdf.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class DirectRenderer: public Renderer {
    uint32_t max_depth;
  public:
    DirectRenderer(uint32_t max_depth):
      max_depth(max_depth) {}
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Rng& rng) const override final;

    static Spectrum uniform_sample_all_lights(const Scene& scene,
        const Ray& ray, const Intersection& isect, const Bsdf& bsdf,
        Rng& rng);
    static Spectrum estimate_direct(const Scene& scene,
        const Ray& ray, const Intersection& isect, const Bsdf& bsdf,
        const Light& light, BxdfFlags bxdf_flags, Rng& rng);
    static Spectrum specular_reflect(const Scene& scene,
        const Renderer& renderer, const Ray& ray, const Intersection& isect,
        const Bsdf& bsdf, uint32_t depth, Rng& rng);
  };
}

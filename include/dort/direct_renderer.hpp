#pragma once
#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class DirectRenderer final: public Renderer {
    uint32_t max_depth;
    std::vector<LightSamplesIdxs> light_samples_idxs;
    std::vector<BsdfSamplesIdxs> bsdf_samples_idxs;
  public:
    DirectRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        uint32_t max_depth):
      Renderer(scene, film, sampler), max_depth(max_depth)
    { }
    virtual Spectrum get_radiance(Ray& ray, uint32_t depth) const override final;
  private:
    virtual void preprocess_() override final;

    struct LightingGeom {
      Point p;
      Normal nn;
      Vector wo;
      float ray_epsilon;
    };

    static Spectrum uniform_sample_all_lights(const Scene& scene,
        const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler,
        slice<const LightSamplesIdxs> light_samples_idxs,
        slice<const BsdfSamplesIdxs> bsdf_samples_idxs);
    static Spectrum uniform_sample_all_lights(const Scene& scene,
        const LightingGeom& geom, const Bsdf& bsdf, Sampler& sampler);
    static Spectrum estimate_direct(const Scene& scene,
        const LightingGeom& geom, const Bsdf& bsdf,
        const Light& light, BxdfFlags bxdf_flags,
        LightSample light_sample, BsdfSample bsdf_sample);
    static Spectrum trace_specular(const Renderer& renderer,
        const LightingGeom& geom, const Bsdf& bsdf,
        BxdfFlags flags, uint32_t depth);
  };
}

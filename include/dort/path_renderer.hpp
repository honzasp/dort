#pragma once
#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/sample_renderer.hpp"

namespace dort {
  class PathRenderer final: public SampleRenderer {
    static constexpr uint32_t MAX_SAMPLES = 4;
    uint32_t max_depth;
    std::vector<SampleIdx> light_idxs;
    std::vector<LightSamplesIdxs> light_samples_idxs;
    std::vector<BsdfSamplesIdxs> bsdf_samples_idxs;
    std::vector<BsdfSamplesIdxs> next_bsdf_samples_idxs;
  public:
    PathRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t max_depth):
      SampleRenderer(scene, film, sampler, camera, iteration_count),
      max_depth(max_depth)
    { }
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void preprocess(CtxG& ctx,
        const Scene& scene, Sampler& sampler) override final;
  };
}

#pragma once
#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class PathRenderer final: public Renderer {
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
        uint32_t max_depth):
      Renderer(scene, film, sampler), max_depth(max_depth)
    { }
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void do_preprocess(const Scene& scene, Sampler& sampler) override final;
  };
}

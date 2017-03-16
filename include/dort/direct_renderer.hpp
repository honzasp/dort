#pragma once
#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/lighting.hpp"
#include "dort/sample_renderer.hpp"

namespace dort {
  class DirectRenderer final: public SampleRenderer {
    uint32_t max_depth;
    std::vector<LightSamplesIdxs> light_samples_idxs;
    std::vector<BsdfSamplesIdxs> bsdf_samples_idxs;
    DirectStrategy strategy;
  public:
    DirectRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t max_depth,
        DirectStrategy strategy):
      SampleRenderer(scene, film, sampler, camera, iteration_count),
      max_depth(max_depth), strategy(strategy)
    { }
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray, Vec2 film_pos,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void preprocess(CtxG& ctx,
        const Scene& scene, Sampler& sampler) override final;
  };
}

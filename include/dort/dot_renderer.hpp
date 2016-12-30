#pragma once
#include "dort/sample_renderer.hpp"

namespace dort {
  class DotRenderer final: public SampleRenderer {
  public:
    DotRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera):
      SampleRenderer(scene, film, sampler, camera) { }

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  protected:
    virtual void preprocess(CtxG& ctx, const Scene& scene,
        Sampler& sampler) override final;
  };
}

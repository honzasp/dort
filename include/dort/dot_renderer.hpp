#pragma once
#include "dort/renderer.hpp"

namespace dort {
  class DotRenderer final: public Renderer {
    uint32_t iteration_count;
  public:
    DotRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count)
    { }
    virtual void render(CtxG& ctx, Progress& progress) override final;
  protected:
    Spectrum get_color(Ray& ray) const;
  };
}

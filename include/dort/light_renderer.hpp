#pragma once
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class LightRenderer final: public Renderer {
    uint32_t iteration_count;
    uint32_t max_depth;
  public:
    LightRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t max_depth):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count),
      max_depth(max_depth)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    Spectrum sample_path(Sampler& sampler, Vec2& out_film_pos) const;
    uint32_t get_job_count(const CtxG& ctx) const;
  };
}

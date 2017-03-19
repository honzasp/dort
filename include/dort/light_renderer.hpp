#pragma once
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class LightRenderer final: public Renderer {
    uint32_t iteration_count;
    uint32_t min_depth;
    uint32_t max_depth;
    DiscreteDistrib1d light_distrib;
  public:
    LightRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t min_depth,
        uint32_t max_depth):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count),
      min_depth(min_depth),
      max_depth(max_depth)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    void sample_path(Sampler& sampler);
    uint32_t get_job_count(const CtxG& ctx) const;
    void add_contrib(Vec2 film_pos, Spectrum contrib);
  };
}

#pragma once
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class LightRenderer final: public Renderer {
    uint32_t iteration_count;
    uint32_t min_length;
    uint32_t max_length;
    DiscreteDistrib1d light_distrib;
  public:
    LightRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t min_length,
        uint32_t max_length):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count),
      min_length(min_length),
      max_length(max_length)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    void sample_path(Sampler& sampler);
    void sample_immediate(const Light& light,
        float light_pick_pdf, Sampler& sampler);
    void connect_to_camera(const Intersection& isect, const Bsdf& bsdf, 
        const Vector& wi, const Spectrum& throughput, Sampler& sampler);
  };
}

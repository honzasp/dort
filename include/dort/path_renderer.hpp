#pragma once
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class PathRenderer final: public Renderer {
  public:
    enum class DirectStrategy {
      SAMPLE_BSDF,
      SAMPLE_LIGHT,
      MIS,
    };
  private:
    uint32_t iteration_count;
    uint32_t min_depth;
    uint32_t max_depth;
    bool only_direct;
    bool sample_all_lights;
    DirectStrategy direct_strategy;
    DiscreteDistrib1d light_distrib;
  public:
    PathRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t min_depth, uint32_t max_depth,
        bool only_direct, bool sample_all_lights,
        DirectStrategy direct_strategy):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count),
      min_depth(min_depth), max_depth(max_depth),
      only_direct(only_direct), sample_all_lights(sample_all_lights),
      direct_strategy(direct_strategy)
    { }
    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    struct LightingGeom {
      Point p;
      float p_epsilon;
      Normal nn;
      Vector wo_camera;
    };

    Spectrum sample(Vec2 film_pos, Sampler& sampler) const;
    Spectrum sample_direct_lighting(const LightingGeom& geom,
        const Bsdf& bsdf, Sampler& sampler) const;

    Spectrum estimate_direct(const LightingGeom& geom,
        const Light& light, const Bsdf& bsdf, Sampler& sampler) const;
    Spectrum estimate_direct_from_bsdf(const LightingGeom& geom,
        const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const;
    Spectrum estimate_direct_from_light(const LightingGeom& geom,
        const Light& light, const Bsdf& bsdf, Sampler& sampler, bool use_mis) const;
  };
}

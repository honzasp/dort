#pragma once
#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class IgiRenderer final: public Renderer {
    struct VirtualLight {
      Point p;
      Normal nn;
      Vector wi;
      float ray_epsilon;
      Spectrum path_contrib;
      std::unique_ptr<Bsdf> bsdf;
    };

    uint32_t max_depth;
    uint32_t max_light_depth;
    std::vector<LightSamplesIdxs> light_samples_idxs;
    std::vector<BsdfSamplesIdxs> bsdf_samples_idxs;
    SampleIdx light_set_idx;

    uint32_t light_set_count;
    uint32_t path_count;
    float g_limit;
    float roulette_threshold;
    std::vector<std::vector<VirtualLight>> light_sets;
  public:
    IgiRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        uint32_t max_depth,
        uint32_t max_light_depth,
        uint32_t light_set_count,
        uint32_t path_count,
        float g_limit,
        float roulette_threshold):
      Renderer(scene, film, sampler),
      max_depth(max_depth),
      max_light_depth(max_light_depth),
      light_set_count(light_set_count),
      path_count(path_count),
      g_limit(g_limit),
      roulette_threshold(roulette_threshold)
    { }
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void do_preprocess(CtxG& ctx,
        const Scene& scene, Sampler& sampler) override final;
    Spectrum sample_virtual_lights(const Scene& scene,
        const LightingGeom& isect_geom, const Bsdf& isect_bsdf, Sampler& sampler) const;
    std::vector<std::vector<VirtualLight>> compute_light_sets(CtxG& ctx,
        const Scene& scene, Sampler& sampler) const;
    void compute_light_path(const Scene& scene,
        const Light& light, float light_pdf,
        const LightRaySample& light_sample,
        std::vector<VirtualLight>& virtual_lights,
        Sampler& sampler) const;
  };
}

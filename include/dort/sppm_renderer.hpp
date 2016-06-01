#pragma once
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/photon_map.hpp"
#include "dort/sample_renderer.hpp"

namespace dort {
  class SppmRenderer final: public SampleRenderer {
    uint32_t max_depth;
    uint32_t max_photon_depth;
    uint32_t photon_path_count;
    PhotonMap photon_map;
  public:
    SppmRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        uint32_t max_depth,
        uint32_t max_photon_depth,
        uint32_t photon_path_count):
      SampleRenderer(scene, film, sampler),
      max_depth(max_depth),
      max_photon_depth(max_photon_depth),
      photon_path_count(photon_path_count)
    { }

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void preprocess(CtxG& ctx,
        const Scene& scene, Sampler& sampler) override final;
  };
}

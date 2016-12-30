#pragma once
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/photon_map.hpp"
#include "dort/recti.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class SppmRenderer final: public Renderer {
  public:
    enum class ParallelMode {
      Automatic,
      SerialIterations,
      ParallelIterations,
    };
  private:
    float initial_radius;
    uint32_t iteration_count;
    uint32_t max_depth;
    uint32_t max_photon_depth;
    uint32_t photon_path_count;
    float alpha;
    ParallelMode parallel_mode;

    DiscreteDistrib1d light_distrib;
    SampleIdx pixel_pos_idx;
  public:
    SppmRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        float initial_radius,
        uint32_t iteration_count,
        uint32_t max_depth,
        uint32_t max_photon_depth,
        uint32_t photon_path_count,
        float alpha,
        ParallelMode parallel_mode):
      Renderer(scene, film, sampler),
      initial_radius(initial_radius),
      iteration_count(iteration_count),
      max_depth(max_depth),
      max_photon_depth(max_photon_depth),
      photon_path_count(photon_path_count),
      alpha(alpha),
      parallel_mode(parallel_mode)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    void iteration_serial(Film& film, Sampler& sampler, float radius) const;
    void iteration_parallel(CtxG& ctx, Film& film, Sampler& sampler, float radius) const;
    void gather_tile(const Film& film, Film& tile_film, Sampler& sampler,
        Recti tile_rect, Recti tile_film_rect,
        const PhotonMap& photon_map, float radius) const;
    Spectrum gather_ray(Ray ray, Sampler& sampler,
        const PhotonMap& photon_map, float radius) const;
    PhotonMap compute_photon_map_serial(Rng& rng) const;
    PhotonMap compute_photon_map_parallel(CtxG& ctx, Rng& rng) const;
    void shoot_photons(std::vector<Photon>& photons, uint32_t count, Rng& rng) const;
  };
}

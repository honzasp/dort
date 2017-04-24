#pragma once
#include <unordered_map>
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/film.hpp"
#include "dort/kd_tree.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"

namespace dort {
  class VcmRenderer final: public Renderer {
  public:
    enum class Mode { VCM, VC, VM };
  private:
    bool use_vc;
    bool use_vm;
    float initial_radius;
    float alpha;
    uint32_t iteration_count;
    uint32_t min_length;
    uint32_t max_length;
    DiscreteDistrib1d light_distrib;
    DiscreteDistrib1d background_light_distrib;
    std::unordered_map<const Light*, float> light_distrib_pdfs;

    mutable std::unordered_map<uint32_t, Film> debug_films;
    std::unordered_map<uint32_t, std::string> debug_names;
    std::string debug_image_dir;
  public:
    VcmRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        Mode mode,
        uint32_t iteration_count,
        float initial_radius,
        float alpha,
        uint32_t min_length,
        uint32_t max_length,
        const std::string& debug_image_dir):
      Renderer(scene, film, sampler, camera),
      use_vc(mode != Mode::VM || iteration_count == 1),
      use_vm(mode != Mode::VC && iteration_count > 1),
      initial_radius(initial_radius), alpha(alpha),
      iteration_count(iteration_count),
      min_length(min_length), max_length(max_length),
      debug_image_dir(debug_image_dir)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:
    struct LightPathState {
      const Light* light;
      float light_pick_pdf;
      Ray ray;
    };

    struct PathVertex {
      Point p;
      float p_epsilon;
      Vector w;
      Normal nn;
      Spectrum throughput;
      std::unique_ptr<Bsdf> bsdf;
      float d_vcm;
      float d_vc;
      float d_vm;
    };

    struct Photon {
      Point p;
      Vector wi;
      Normal nn;
      Spectrum throughput;
      float d_vcm;
      float d_vc;
      float d_vm;
      uint32_t bounces;
    };

    struct PhotonKdTraits {
      using Element = Photon;
      static Point element_point(const Element& elem) {
        return elem.p;
      }
    };

    struct IterationState {
      uint32_t idx;
      uint32_t path_count;
      float radius;
      float mis_vm_weight;
      float mis_vc_weight;
      float vm_normalization;
      KdTree<PhotonKdTraits> photon_tree;
    };

    std::vector<Photon> iteration(CtxG& ctx, uint32_t idx,
        std::vector<Photon> prev_photons);

    LightPathState light_walk(const IterationState& iter_state,
        std::vector<PathVertex>& light_vertices,
        std::vector<Photon>& photons, Sampler& sampler);
    Spectrum camera_walk(const IterationState& iter_state,
        const LightPathState& light_path,
        const std::vector<PathVertex>& light_vertices,
        Vec2 film_pos, Sampler& sampler);

    void connect_to_camera(const IterationState& iter_state,
        const PathVertex& y, uint32_t bounces, Film& film, Sampler& sampler) const;
    Spectrum connect_to_background_light(const Ray& camera_ray,
        const PathVertex& zp, Spectrum throughput,
        Vec2 film_pos, uint32_t bounces, Sampler& sampler) const;
    Spectrum merge_with_photons(const IterationState& iter_state,
        const PathVertex& z, Vec2 film_pos, uint32_t bounces) const;
    Spectrum connect_to_light_vertices(const IterationState& iter_state,
        const PathVertex& z, const std::vector<PathVertex>& light_vertices,
        Vec2 film_pos, uint32_t bounces) const;
    Spectrum connect_to_light(const IterationState& iter_state,
        const LightPathState& light_path, const PathVertex& z,
        Vec2 film_pos, uint32_t bounces, Sampler& sampler) const;
    Spectrum connect_area_light(const PathVertex& zp, const PathVertex& z,
        const Light* area_light, Vec2 film_pos, uint32_t bounces) const;

    void init_debug_films();
    void save_debug_films();
    void store_debug_weighted_contrib(uint32_t s, uint32_t t, bool is_vm,
        Vec2 film_pos, const Spectrum& contrib, float weight) const;
    void store_debug_contrib(uint32_t s, uint32_t t, bool is_vm, bool is_weighted,
        Vec2 film_pos, const Spectrum& contrib) const;
    uint32_t debug_image_key(uint32_t s, uint32_t t, bool is_vm, bool is_weighted) const;
  };
}

#pragma once
#include <unordered_map>
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/film.hpp"
#include "dort/light.hpp"
#include "dort/renderer.hpp"
#include "dort/slice.hpp"

namespace dort {
  /// A bidirectional path renderer.
  /// This renderer samples vertices both from light and the camera and combines
  /// the various sub-paths using multiple importance sampling.
  class BdptRenderer final: public Renderer {
    uint32_t iteration_count;
    uint32_t min_depth;
    uint32_t max_depth;
    bool use_t1_paths;
    DiscreteDistrib1d light_distrib;
    DiscreteDistrib1d background_light_distrib;
    std::unordered_map<const Light*, float> light_distrib_pdfs;

    mutable std::unordered_map<uint32_t, Film> debug_films;
    std::string debug_image_dir;
  public:
    BdptRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count,
        uint32_t min_depth,
        uint32_t max_depth,
        bool use_t1_paths,
        const std::string& debug_image_dir):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count),
      min_depth(min_depth), max_depth(max_depth),
      use_t1_paths(use_t1_paths),
      debug_image_dir(debug_image_dir)
    { }

    virtual void render(CtxG& ctx, Progress& progress) override final;
  private:

    struct Vertex {
      Point p;
      float p_epsilon;
      Normal nn;
      std::unique_ptr<Bsdf> bsdf;

      /// Stores an area light hit on the camera path.
      const Light* area_light;

      /// Stores a background light if the camera walk escaped the scene.
      /// The last vertex on the camera path may represent a "virtual vertex"
      /// that was created when the last camera ray escaped the scene and "hit"
      /// a background light.
      const Light* background_light;

      /// Stores some form of pdf of being sampled from the previous vertex on
      /// the same subpath.
      /// - y0 on light_walk: area pdf of ray origin (for distant lights, the area
      ///   is a plane perpendicular to the ray direction)
      /// - y1 on light_walk: area pdf of the ray hit point (for non-distant
      ///   lights) or solid angle pdf of the ray origin (for distant)
      /// - y0 freshly sampled from path_contrib(): area pdf of y0 (for
      ///   non-distant) or solid angle pdf of y1->y0 (for distant lights)
      /// - yi for i >= 2: area pdf of sampling the BSDF at the previous vertex
      ///
      /// - z0 on camera_walk: area pdf of the ray origin
      /// - z1 on camera_walk: area pdf of the ray hit point
      /// - zi on camera_walk: area pdf of sampling the BSDF at previous vertex
      /// - z(t-1) on camera_walk if background_light != nullptr: solid angle
      ///   pdf of sampling from the BSDF at previous vertex
      /// - z0 sampled from path_contrib(): area pdf of z0 sampled from z1
      float fwd_pdf; 

      /// Stores the pdf of being sampled from the next vertex in the walk.
      /// - yi on light_walk: area pdf of being sampled from y(i+1) with BSDF
      /// - zi on camera_walk: area pdf of begin sampled from z(i+1) with BSDF
      float bwd_pdf;

      Spectrum alpha;
      bool is_delta;
    };

    void preprocess(const Scene& scene);
    void postprocess();
    void render_tile(CtxG& ctx, Recti tile_rect, Recti tile_film_rect,
        Film& tile_film, Sampler& sampler, Progress& progress) const;
    void iteration(Film& film, uint32_t iteration);

    Spectrum sample_path(const Scene& scene, Vec2 film_pos, Sampler& sampler) const;
    std::vector<Vertex> random_light_walk(const Scene& scene,
        const Light*& out_light, Rng& rng) const;
    std::vector<Vertex> random_camera_walk(const Scene& scene,
        Vec2 film_pos, Rng& rng) const;
    Spectrum path_contrib(const Scene& scene, Vec2 film_res,
        const Light& light, const Camera& camera, Rng& rng,
        const std::vector<Vertex>& light_walk,
        const std::vector<Vertex>& camera_walk,
        uint32_t s, uint32_t t,
        const Light*& out_light,
        Vertex& out_first_light,
        Vertex& out_first_camera,
        Vec2& out_film_pos) const;
    float path_weight(const Scene& scene, Vec2 film_res,
        const Light& light, const Camera& camera,
        const std::vector<Vertex>& light_walk,
        const std::vector<Vertex>& camera_walk,
        uint32_t s, uint32_t t,
        const Vertex& first_light,
        const Vertex& first_camera) const;

    void init_debug_films();
    void save_debug_films();
    void store_debug_contrib(uint32_t s, uint32_t t, bool weighted,
        Vec2 film_pos, const Spectrum& contrib) const;
  };
}

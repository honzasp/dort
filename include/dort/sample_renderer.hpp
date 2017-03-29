#pragma once
#include "dort/ctx.hpp"
#include "dort/rect_i.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  class SampleRenderer: public Renderer {
    SampleIdx pixel_pos_idx;
  protected:
    uint32_t iteration_count;
  public:
    SampleRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera,
        uint32_t iteration_count):
      Renderer(scene, film, sampler, camera),
      iteration_count(iteration_count)
    { }
    virtual void render(CtxG& ctx, Progress& progress) override final;

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray, Vec2 film_pos,
        uint32_t depth, Sampler& sampler) const = 0;
  protected:
    virtual void preprocess(CtxG& ctx, const Scene& scene, Sampler& sampler) = 0;
    virtual void postprocess(CtxG& ctx) {
      (void)ctx;
    }
    virtual void iteration(Film& film, uint32_t iteration) {
      (void)film; (void)iteration;
    }
  private:
    void render_tile(CtxG& ctx, Recti tile_rect, Recti tile_film_rect,
        Film& tile_film, Sampler& sampler, Progress& progress) const;
  };
}

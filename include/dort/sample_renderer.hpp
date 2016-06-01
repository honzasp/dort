#pragma once
#include "dort/ctx.hpp"
#include "dort/recti.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  class SampleRenderer: public Renderer {
    SampleIdx pixel_pos_idx;
  public:
    SampleRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler):
      Renderer(scene, film, sampler) { }
    virtual void render(CtxG& ctx) override final;

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const = 0;
  protected:
    virtual void preprocess(CtxG& ctx, const Scene& scene, Sampler& sampler) = 0;
  private:
    void render_tile(CtxG& ctx, Recti tile_rect,
        Recti tile_film_rect, Film& tile_film, Sampler& sampler) const;
  };
}

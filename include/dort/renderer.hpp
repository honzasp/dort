#pragma once
#include "dort/ctx.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/recti.hpp"
#include "dort/sampler.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  class Renderer {
    SampleIdx pixel_pos_idx;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Film> film;
    std::shared_ptr<Sampler> sampler;
  public:
    Renderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler):
      scene(scene), film(film), sampler(sampler) { }

    virtual ~Renderer() {}
    void preprocess(CtxG& ctx);
    void render(CtxG& ctx);

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const = 0;
  protected:
    virtual void do_preprocess(CtxG& ctx, const Scene& scene, Sampler& sampler) = 0;
    static DiscreteDistrib1d compute_light_distrib(const Scene& scene);
  private:
    void render_tile(CtxG& ctx, Recti tile_rect,
        Recti tile_film_rect, Film& tile_film, Sampler& sampler) const;
    Vec2i layout_tiles(CtxG& ctx) const;
  };
}

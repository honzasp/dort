#pragma once
#include "dort/dort.hpp"

namespace dort {
  class Progress {
  public:
    virtual bool is_cancelled() const = 0;
    virtual void set_percent_done(float done) = 0;
  };

  class Renderer {
  protected:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Film> film;
    std::shared_ptr<Sampler> sampler;
    std::shared_ptr<Camera> camera;
  public:
    Renderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        std::shared_ptr<Camera> camera):
      scene(scene), film(film), sampler(sampler), camera(camera) { }
    virtual ~Renderer() {}
    virtual void render(CtxG& ctx, Progress& progress) = 0;
  protected:
    static Vec2i layout_tiles(const CtxG& ctx, Vec2i film_res);
    void iteration_tiled(CtxG& ctx,
        std::function<Spectrum(Vec2i, Vec2&, Sampler&)> render_pixel);
    void iteration_tiled_per_job(CtxG& ctx,
        std::function<void(Film&, Recti, Recti, Sampler&)> render_tile);
  };
}

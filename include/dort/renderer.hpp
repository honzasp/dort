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
  public:
    Renderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler):
      scene(scene), film(film), sampler(sampler) { }
    virtual ~Renderer() {}
    virtual void render(CtxG& ctx, Progress& progress) = 0;

    static Vec2i layout_tiles(const CtxG& ctx,
        const Film& film, const Sampler& sampler);
  };
}

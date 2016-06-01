#pragma once
#include "dort/dort.hpp"

namespace dort {
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
    virtual void render(CtxG& ctx) = 0;

    static Vec2i layout_tiles(const CtxG& ctx,
        const Film& film, const Sampler& sampler);
  };
}

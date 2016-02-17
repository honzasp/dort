#pragma once
#include "dort/sampler.hpp"

namespace dort {
  class Renderer {
    SampleIdx pixel_pos_idx;
  public:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Film> film;
    std::shared_ptr<Sampler> sampler;

    Renderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler):
      scene(scene), film(film), sampler(sampler) { }

    virtual ~Renderer() {}
    void preprocess();
    void render();

    virtual Spectrum get_radiance(Ray& ray, uint32_t depth) const = 0;
  protected:
    virtual void preprocess_() = 0;
  };
}

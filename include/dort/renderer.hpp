#pragma once
#include "dort/sampler.hpp"

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
    void preprocess();
    void render();

    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const = 0;
  protected:
    virtual void preprocess_(const Scene& scene, Sampler& sampler) = 0;
  };
}

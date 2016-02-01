#pragma once
#include "dort/film.hpp"
#include "dort/geometry.hpp"
#include "dort/scene.hpp"

namespace dort {
  class Renderer {
  public:
    virtual ~Renderer() {}
    void render(const Scene& scene, Film& film);
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth = 0) const = 0;
  };
}

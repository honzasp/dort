#pragma once
#include "dort/dort.hpp"

namespace dort {
  class Renderer {
  public:
    virtual ~Renderer() {}
    void render(const Scene& scene, Film& film, Rng& rng);
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Rng& rng) const = 0;
  };
}

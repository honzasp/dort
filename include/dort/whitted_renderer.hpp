#pragma once
#include "dort/renderer.hpp"

namespace dort {
  class WhittedRenderer: public Renderer {
    uint32_t max_depth;
  public:
    WhittedRenderer(uint32_t max_depth):
      max_depth(max_depth) {}
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray, uint32_t depth)
      const override final;
  };
}

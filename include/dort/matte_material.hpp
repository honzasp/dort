#pragma once
#include "dort/material.hpp"

namespace dort {
  class MatteMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflectance;
    std::shared_ptr<TextureGeom<float>> sigma;
  public:
    MatteMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> reflectance,
        std::shared_ptr<TextureGeom<float>> sigma):
      reflectance(reflectance), sigma(sigma)
    { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& geom) const override final;
  };
}

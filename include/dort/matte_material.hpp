#pragma once
#include "dort/material.hpp"

namespace dort {
  class MatteMaterial: public Material {
    std::shared_ptr<Texture<Spectrum>> reflectance;
    std::shared_ptr<Texture<float>> sigma;
  public:
    MatteMaterial(
        std::shared_ptr<Texture<Spectrum>> reflectance,
        std::shared_ptr<Texture<float>> sigma):
      reflectance(reflectance), sigma(sigma)
    { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

#pragma once
#include "dort/material.hpp"

namespace dort {
  class MatteMaterial: public Material {
    Spectrum reflectance;
    float sigma;
  public:
    MatteMaterial(const Spectrum& reflectance, float sigma = 0.f):
      reflectance(reflectance), sigma(sigma)
    { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

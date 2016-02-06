#pragma once
#include "dort/material.hpp"

namespace dort {
  class MatteMaterial: public Material {
    Spectrum reflectance;
    float roughness;
  public:
    MatteMaterial(const Spectrum& reflectance, float roughness = 0.f):
      reflectance(reflectance), roughness(roughness)
    { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

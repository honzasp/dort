#pragma once
#include "dort/material.hpp"

namespace dort {
  class PlasticMaterial: public Material {
    Spectrum diffuse;
    Spectrum reflection;
    float roughness;
    float eta;
  public:
    PlasticMaterial(const Spectrum& diffuse, const Spectrum& reflection,
        float roughness, float eta = 1.5):
      diffuse(diffuse), reflection(reflection),
      roughness(roughness), eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& diff_geom) const override final;
  };
}

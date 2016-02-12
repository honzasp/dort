#pragma once
#include "dort/material.hpp"

namespace dort {
  class MirrorMaterial final: public Material {
    Spectrum reflectance;
  public:
    MirrorMaterial(const Spectrum& reflectance):
      reflectance(reflectance)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) 
      const override final;
  };

  class GlassMaterial final: public Material {
    Spectrum reflectance;
    Spectrum transmittance;
    float eta;
  public:
    GlassMaterial(const Spectrum& reflectance,
        const Spectrum& transmittance, float eta):
      reflectance(reflectance),
      transmittance(transmittance),
      eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

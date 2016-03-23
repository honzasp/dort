#pragma once
#include "dort/material.hpp"

namespace dort {
  class MirrorMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflectance;
  public:
    MirrorMaterial(std::shared_ptr<TextureGeom<Spectrum>> reflectance):
      reflectance(reflectance)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) 
      const override final;
  };

  class GlassMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflectance;
    std::shared_ptr<TextureGeom<Spectrum>> transmittance;
    std::shared_ptr<TextureGeom<float>> eta;
  public:
    GlassMaterial(std::shared_ptr<TextureGeom<Spectrum>> reflectance,
        std::shared_ptr<TextureGeom<Spectrum>> transmittance,
        std::shared_ptr<TextureGeom<float>> eta):
      reflectance(reflectance),
      transmittance(transmittance),
      eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

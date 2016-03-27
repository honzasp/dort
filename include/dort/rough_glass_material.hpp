#pragma once
#include "dort/material.hpp"

namespace dort {
  class RoughGlassMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflectance;
    std::shared_ptr<TextureGeom<Spectrum>> transmittance;
    std::shared_ptr<TextureGeom<float>> roughness;
    std::shared_ptr<TextureGeom<float>> eta;
  public:
    RoughGlassMaterial(std::shared_ptr<TextureGeom<Spectrum>> reflectance,
        std::shared_ptr<TextureGeom<Spectrum>> transmittance,
        std::shared_ptr<TextureGeom<float>> roughness,
        std::shared_ptr<TextureGeom<float>> eta):
      reflectance(reflectance),
      transmittance(transmittance),
      roughness(roughness), eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& shading_geom, const Normal& nn_geom) const override final;
  };
}

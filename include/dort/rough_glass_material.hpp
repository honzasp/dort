#pragma once
#include "dort/material.hpp"

namespace dort {
  class RoughGlassMaterial final: public Material {
    std::shared_ptr<Texture<Spectrum>> reflectance;
    std::shared_ptr<Texture<Spectrum>> transmittance;
    std::shared_ptr<Texture<float>> roughness;
    std::shared_ptr<Texture<float>> eta;
  public:
    RoughGlassMaterial(std::shared_ptr<Texture<Spectrum>> reflectance,
        std::shared_ptr<Texture<Spectrum>> transmittance,
        std::shared_ptr<Texture<float>> roughness,
        std::shared_ptr<Texture<float>> eta):
      reflectance(reflectance),
      transmittance(transmittance),
      roughness(roughness), eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

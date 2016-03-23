#pragma once
#include "dort/material.hpp"

namespace dort {
  class PlasticMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> diffuse;
    std::shared_ptr<TextureGeom<Spectrum>> reflection;
    std::shared_ptr<TextureGeom<float>> roughness;
    std::shared_ptr<TextureGeom<float>> eta;
  public:
    PlasticMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> diffuse,
        std::shared_ptr<TextureGeom<Spectrum>> reflection,
        std::shared_ptr<TextureGeom<float>> roughness,
        std::shared_ptr<TextureGeom<float>> eta):
      diffuse(diffuse), reflection(reflection),
      roughness(roughness), eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& diff_geom) const override final;
  };
}

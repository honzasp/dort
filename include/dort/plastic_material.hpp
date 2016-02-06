#pragma once
#include "dort/material.hpp"

namespace dort {
  class PlasticMaterial: public Material {
    std::shared_ptr<Texture<Spectrum>> diffuse;
    std::shared_ptr<Texture<Spectrum>> reflection;
    std::shared_ptr<Texture<float>> roughness;
    std::shared_ptr<Texture<float>> eta;
  public:
    PlasticMaterial(
        std::shared_ptr<Texture<Spectrum>> diffuse,
        std::shared_ptr<Texture<Spectrum>> reflection,
        std::shared_ptr<Texture<float>> roughness,
        std::shared_ptr<Texture<float>> eta):
      diffuse(diffuse), reflection(reflection),
      roughness(roughness), eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& diff_geom) const override final;
  };
}

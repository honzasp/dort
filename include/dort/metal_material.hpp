#pragma once
#include "dort/material.hpp"

namespace dort {
  class MetalMaterial final: public Material {
    std::shared_ptr<Texture<Spectrum>> reflection;
    std::shared_ptr<Texture<float>> roughness;
    std::shared_ptr<Texture<float>> eta;
    std::shared_ptr<Texture<float>> k;
  public:
    MetalMaterial(
        std::shared_ptr<Texture<Spectrum>> reflection,
        std::shared_ptr<Texture<float>> roughness,
        std::shared_ptr<Texture<float>> eta,
        std::shared_ptr<Texture<float>> k):
      reflection(reflection), roughness(roughness), eta(eta), k(k)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& diff_geom) const override final;
  };
}

#pragma once
#include "dort/material.hpp"

namespace dort {
  class PhongMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> k_diffuse;
    std::shared_ptr<TextureGeom<Spectrum>> k_glossy;
    std::shared_ptr<TextureGeom<float>> exponent;
  public:
    PhongMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> k_diffuse,
        std::shared_ptr<TextureGeom<Spectrum>> k_glossy,
        std::shared_ptr<TextureGeom<float>> exponent):
      k_diffuse(k_diffuse), k_glossy(k_glossy), exponent(exponent)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& geom) const override final;
  };
}


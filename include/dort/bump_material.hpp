#pragma once
#include "dort/material.hpp"

namespace dort {
  class BumpMaterial final: public Material {
    std::shared_ptr<TextureGeom<float>> displac;
    std::shared_ptr<Material> material;
  public:
    BumpMaterial(
        std::shared_ptr<TextureGeom<float>> displac,
        std::shared_ptr<Material> material):
      displac(displac), material(material)
    { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& geom) const override final;
  };
}


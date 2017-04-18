#pragma once
#include "dort/material.hpp"
#include "dort/texture.hpp"

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

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}


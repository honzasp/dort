#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  class Material {
  public:
    virtual ~Material() { }
    std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& geom) const {
      auto bsdf = std::make_unique<Bsdf>(geom);
      this->add_bxdfs(geom, Spectrum(1.f), *bsdf);
      return bsdf;
    }

    virtual void add_bxdfs(const DiffGeom& geom, Spectrum scale, Bsdf& bsdf) const = 0;
  };
}

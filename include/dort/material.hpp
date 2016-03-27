#pragma once
#include "dort/dort.hpp"

namespace dort {
  class Material {
  public:
    virtual ~Material() { }
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& shading_geom, const Normal& nn_geom) const = 0;
    std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& shading_geom) const;
  };
}

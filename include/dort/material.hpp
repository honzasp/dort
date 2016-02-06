#pragma once
#include "dort/dort.hpp"


namespace dort {
  class Material {
  public:
    virtual ~Material() { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) const = 0;
  };
}

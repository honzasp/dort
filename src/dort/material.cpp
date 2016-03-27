#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/shape.hpp"

namespace dort {
  std::unique_ptr<Bsdf> Material::get_bsdf(const DiffGeom& shading_geom) const {
    return this->get_bsdf(shading_geom, shading_geom.nn);
  }
}

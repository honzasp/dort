#pragma once
#include <memory>
#include <vector>
#include "dort/primitive.hpp"

namespace dort {
  class ListPrimitive: public Primitive {
    std::vector<std::unique_ptr<Primitive>> primitives;
    Box total_bound;
  public:
    ListPrimitive(std::vector<std::unique_ptr<Primitive>> prims);
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual Box bound() const override final;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const override final;
  };
}



#pragma once
#include <memory>
#include <vector>
#include "dort/primitive.hpp"

namespace dort {
  class ListPrimitive final: public Primitive {
    std::vector<std::unique_ptr<Primitive>> prims;
    std::vector<Box> prim_bounds;
    Box total_bounds;
  public:
    ListPrimitive(std::vector<std::unique_ptr<Primitive>> prims);
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
  };
}



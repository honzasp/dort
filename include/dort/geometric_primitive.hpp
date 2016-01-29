#pragma once
#include <memory>
#include "dort/primitive.hpp"

namespace dort {
  class GeometricPrimitive: public Primitive {
    std::shared_ptr<Shape> shape;
    Spectrum color;
  public:
    GeometricPrimitive(std::shared_ptr<Shape> shape, Spectrum color);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const final;
    virtual Box bounds() const final;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const final;
  };
}

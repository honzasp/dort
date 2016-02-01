#pragma once
#include <memory>
#include "dort/primitive.hpp"

namespace dort {
  class GeometricPrimitive: public Primitive {
    std::shared_ptr<Shape> shape;
    Spectrum color;
    float reflection;
  public:
    GeometricPrimitive(std::shared_ptr<Shape> shape,
        Spectrum color, float reflection = 0.f);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const final override;
    virtual Box bounds() const override final;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const override final;
    virtual float get_reflection(const DiffGeom& diff_geom) const override final;
  };
}

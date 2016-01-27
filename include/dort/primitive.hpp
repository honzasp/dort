#pragma once
#include "dort/shape.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  struct Intersection {
    float ray_epsilon;
    DiffGeom diff_geom;
    const Primitive* primitive;
  };

  class Primitive {
  public:
    Primitive() { };
    virtual ~Primitive() { };
    virtual bool intersect(Ray& ray, Intersection& out_isect) const = 0;
    virtual Box bound() const = 0;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const = 0;
  };
}

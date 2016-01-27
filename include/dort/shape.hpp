#pragma once
#include "dort/dort.hpp"
#include "dort/geometry.hpp"

namespace dort {
  struct DiffGeom {
    Point p;
    Normal nn;
  };

  class Shape {
  public:
    Shape() {}
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const = 0;
    virtual Box bound() const = 0;
  };
}

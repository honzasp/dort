#pragma once
#include <memory>
#include "dort/shape.hpp"

namespace dort {
  class Sphere: public Shape {
    float radius;
  public:
    Sphere(float radius);
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const final;
    virtual Box bound() const final;
  };
}

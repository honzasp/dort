#pragma once
#include "dort/shape.hpp"

namespace dort {
  class Sphere: public Shape {
    float radius;
  public:
    Sphere(float radius);
    virtual bool hit(const Ray& ray, Hit& out_hit) const final;
  };
}

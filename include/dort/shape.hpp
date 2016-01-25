#pragma once
#include "dort/dort.hpp"
#include "dort/geometry.hpp"

namespace dort {
  struct Hit {
    float t;
    float ray_epsilon;
    Point p;
    Vector normal;
  };

  class Shape {
  public:
    virtual bool hit(const Ray& ray, Hit& out_hit) const = 0;;
  };
}

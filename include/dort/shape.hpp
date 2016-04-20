#pragma once
#include "dort/box.hpp"
#include "dort/geometry.hpp"

namespace dort {
  struct DiffGeom {
    Point p;
    Normal nn;
    float u, v;
    Vector dpdu, dpdv;
  };

  class Shape {
  public:
    Shape() {}
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const = 0;
    virtual bool hit_p(const Ray& ray) const = 0;
    virtual Box bounds() const = 0;

    virtual float area() const = 0;
    virtual Point sample_point(float u1, float u2, Normal& out_n) const = 0;
    virtual float point_pdf(const Point& pt) const = 0;
    virtual Point sample_point_eye(const Point& eye,
        float u1, float u2, Normal& out_n) const;
    virtual float point_eye_pdf(const Point& eye, const Vector& w) const;
  };
}

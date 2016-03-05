#pragma once
#include "dort/shape.hpp"

namespace dort {
  class DiskShape final: public Shape {
    float radius;
    float inv_radius;
    float z_coord;
  public:
    DiskShape(float radius, float z_coord = 0.f):
      radius(radius), inv_radius(1.f / radius), z_coord(z_coord) { }

    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual float area() const override final;
    virtual Point sample_point(float u1, float u2, Normal& out_n) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  };
}
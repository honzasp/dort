#pragma once
#include "dort/shape.hpp"

namespace dort {
  class CylinderShape final: public Shape {
    float radius;
    float z_min;
    float z_max;
  public:
    CylinderShape(float radius, float z_min, float z_max);
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const final;
    virtual float area() const override final;

    virtual Point sample_point(Vec2 uv,
        float& out_pos_pdf, Normal& out_n, float& out_ray_epsilon) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  private:
    bool solve_hit_t(const Ray& ray, float& out_t_hit) const;
  };
}

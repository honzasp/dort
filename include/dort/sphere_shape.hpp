#pragma once
#include <memory>
#include "dort/shape.hpp"

namespace dort {
  class SphereShape final: public Shape {
    float radius;
  public:
    SphereShape(float radius);
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const final;
    virtual float area() const override final;

    virtual Point sample_point(Vec2 uv,
        float& out_pos_pdf, Normal& out_n, float& out_ray_epsilon) const override final;
    virtual Point sample_point_pivot(const Point& pivot, Vec2 uv,
        float& out_dir_pdf, Normal& out_n, float& out_ray_epsilon) const override final;
    virtual float point_pdf(const Point& pt) const override final;
    virtual float point_pivot_pdf(const Point& pivot,
        const Vector& w) const override final;
  private:
    bool solve_hit_t(const Ray& ray, float& out_t_hit) const;
  };
}

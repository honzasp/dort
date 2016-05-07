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
    virtual Point sample_point(float u1, float u2,
        Normal& out_n, float& out_ray_epsilon) const override final;
    virtual float point_pdf(const Point& pt) const override final;
    virtual Point sample_point_eye(const Point& eye,
        float u1, float u2, Normal& out_n) const override final;
    virtual float point_eye_pdf(const Point& eye, const Vector& w) const override final;
  private:
    bool solve_hit_t(const Ray& ray, float& out_t_hit) const;
  };
}

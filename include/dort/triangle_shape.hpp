#pragma once
#include "dort/shape.hpp"

namespace dort {
  class TriangleShape final: public Shape {
    const Mesh* mesh;
    uint32_t index;
  public:
    TriangleShape(const Mesh* mesh, uint32_t index):
      mesh(mesh), index(index) { }

    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual float area() const override final;

    virtual Point sample_point(Vec2 uv,
        float& out_pos_pdf, Normal& out_n, float& out_ray_epsilon) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  };
}

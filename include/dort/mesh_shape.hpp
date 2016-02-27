#pragma once
#include "dort/discrete_distrib_1d.hpp"
#include "dort/shape.hpp"

namespace dort {
  class MeshShape final: public Shape {
    const Mesh* mesh;
    DiscreteDistrib1d area_distrib;
  public:
    MeshShape(const Mesh* mesh);

    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual float area() const override final;
    virtual Point sample_point(float u1, float u2, Normal& out_n) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  };
}

#pragma once
#include "dort/geometry.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  struct Triangle final {
    std::array<Point, 3> p;
    std::array<Vec2, 3> uv;

    Triangle(const TriangleMesh* mesh, uint32_t index);

    bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const;
    bool hit_p(const Ray& ray) const;
    Box bounds() const;

    float area() const;
    Point sample_point(float u1, float u2, Normal& out_n) const;
    float point_pdf(const Point& pt) const;
  };
}

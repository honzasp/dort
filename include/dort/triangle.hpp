#pragma once
#include "dort/geometry.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  struct Triangle {
    std::array<Point, 3> p;

    Triangle(const Mesh& mesh, uint32_t index);
    bool hit_p(const Ray& ray) const;
    Box bounds() const;
    float area() const;
    Point sample_point(Vec2 uv, float& out_pos_pdf,
        Normal& out_n, float& out_ray_epsilon) const;
    float point_pdf(const Point& pt) const;
  };

  struct TriangleUv: public Triangle {
    std::array<Vec2, 3> uv;
    std::array<Normal, 3> n;
    bool has_shading_normals;

    TriangleUv(const Mesh& mesh, uint32_t index);
    bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const;
  };
}

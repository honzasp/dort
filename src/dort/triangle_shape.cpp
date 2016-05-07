#include "dort/box.hpp"
#include "dort/triangle.hpp"
#include "dort/triangle_shape.hpp"

namespace dort {
  bool TriangleShape::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    return TriangleUv(*this->mesh, this->index).hit(ray, out_t_hit,
        out_ray_epsilon, out_diff_geom);
  }

  bool TriangleShape::hit_p(const Ray& ray) const {
    return Triangle(*this->mesh, this->index).hit_p(ray);
  }

  Box TriangleShape::bounds() const {
    return Triangle(*this->mesh, this->index).bounds();
  }

  float TriangleShape::area() const {
    return Triangle(*this->mesh, this->index).area();
  }

  Point TriangleShape::sample_point(float u1, float u2,
      Normal& out_n, float& out_ray_epsilon) const 
  {
    return Triangle(*this->mesh, this->index).sample_point(
        u1, u2, out_n, out_ray_epsilon);
  }

  float TriangleShape::point_pdf(const Point& pt) const {
    return Triangle(*this->mesh, this->index).point_pdf(pt);
  }
}

#include "dort/shape.hpp"

namespace dort {
  Point Shape::sample_point_pivot(const Point& pivot, Vec2 uv,
      float& out_dir_pdf, Normal& out_n, float& out_ray_epsilon) const
  {
    float pos_pdf;
    Point pt = this->sample_point(uv, pos_pdf, out_n, out_ray_epsilon);
    out_dir_pdf = pos_pdf * length_squared(pivot - pt)
      / abs_dot(out_n, normalize(pivot - pt));
    return pt;
  }

  float Shape::point_pivot_pdf(const Point& pivot, const Vector& w) const {
    DiffGeom diff_geom;
    float t_hit;
    float ray_epsilon;
    Ray ray(pivot, w, 0.f);

    if(!this->hit(ray, t_hit, ray_epsilon, diff_geom)) {
      return 0.f;
    }

    Point pt = ray.point_t(t_hit);
    return this->point_pdf(pt) * length_squared(pt - pivot) / abs_dot(w, diff_geom.nn);
  }
}

#include "dort/shape.hpp"

namespace dort {
  Point Shape::sample_point_eye(const Point& eye,
      float u1, float u2, Normal& out_n) const
  {
    (void)eye;
    return this->sample_point(u1, u2, out_n);
  }

  float Shape::point_eye_pdf(const Point& eye, const Vector& w) const {
    DiffGeom diff_geom;
    float t_hit;
    float ray_epsilon;
    Ray ray(eye, w, 0.f);
    if(!this->hit(ray, t_hit, ray_epsilon, diff_geom)) {
      return 0.f;
    }

    return length_squared(ray.point_t(t_hit) - eye) /
      (abs_dot(w, diff_geom.nn) * this->area());
  }
}

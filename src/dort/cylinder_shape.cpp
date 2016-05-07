#include "dort/cylinder_shape.hpp"

namespace dort {
  CylinderShape::CylinderShape(float radius, float z_min, float z_max):
    radius(radius),
    z_min(z_min),
    z_max(z_max)
  { }

  bool CylinderShape::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    float t_hit;
    if(!this->solve_hit_t(ray, t_hit)) {
      return false;
    }

    Point p_hit = ray.point_t(t_hit);
    float x = p_hit.v.x;
    float y = p_hit.v.y;
    float z = p_hit.v.z;

    float phi = atan(y, x);
    if(phi < 0.f) {
      phi = phi + TWO_PI;
    }

    out_diff_geom.p = p_hit;
    out_diff_geom.nn = Normal(x, y, 0.f) / this->radius;
    out_diff_geom.u = phi * INV_TWO_PI;
    out_diff_geom.v = (z - this->z_min) / (this->z_max - this->z_min);
    out_diff_geom.dpdu = Vector(-TWO_PI * y, TWO_PI * x, 0.f);
    out_diff_geom.dpdv = Vector(0.f, 0.f, this->z_max - this->z_min);
    out_t_hit = t_hit;
    out_ray_epsilon = 5e-3f * abs(t_hit);
    return true;
  }

  bool CylinderShape::hit_p(const Ray& ray) const {
    float t_hit;
    return this->solve_hit_t(ray, t_hit);
  }

  Box CylinderShape::bounds() const {
    return Box(Point(-this->radius, -this->radius, this->z_min),
        Point(this->radius, this->radius, this->z_max));
  }

  float CylinderShape::area() const {
    return TWO_PI * this->radius * (this->z_max - this->z_min);
  }

  Point CylinderShape::sample_point(float u1, float u2,
      Normal& out_n, float& out_ray_epsilon) const 
  {
    float z = lerp(u1, this->z_min, this->z_max);
    float phi = TWO_PI * u2;
    float cos_phi = cos(phi);
    float sin_phi = sin(phi);
    out_n = Normal(cos_phi, sin_phi, 0.f);
    out_ray_epsilon = 5e-3f;
    return Point(cos_phi * this->radius, sin_phi * this->radius, z);
  }

  float CylinderShape::point_pdf(const Point&) const {
    return 1.f / this->area();
  }

  bool CylinderShape::solve_hit_t(const Ray& ray, float& out_t_hit) const {
    const auto& o = ray.orig.v;
    const auto& d = ray.dir.v;
    float a = square(d.x) + square(d.y);
    float b = 2.f * (o.x * d.x + o.y * d.y);
    float c = square(o.x) + square(o.y) - square(this->radius);
    float t0, t1;
    if(!solve_quadratic(a, b, c, t0, t1)) {
      return false;
    }

    if(t1 < t0) {
      std::swap(t0, t1);
    }

    if(ray.t_min <= t0 && t0 <= ray.t_max) {
      float z0 = o.z + d.z * t0;
      if(this->z_min <= z0 && z0 <= this->z_max) {
        out_t_hit = t0;
        return true;
      }
    }

    if(ray.t_min <= t1 && t1 <= ray.t_max) {
      float z1 = o.z + d.z * t1;
      if(this->z_min <= z1 && z1 <= this->z_max) {
        out_t_hit = t1;
        return true;
      }
    }

    return false;
  }
}

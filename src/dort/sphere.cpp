#include <utility>
#include "dort/sphere.hpp"

namespace dort {
  Sphere::Sphere(float radius):
    radius(radius), inv_radius(1.f / radius)
  { }

  bool Sphere::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    float A = dot(ray.dir.v, ray.dir.v);
    float B = 2.f * dot(ray.orig.v, ray.dir.v);
    float C = dot(ray.orig.v, ray.orig.v) - this->radius * this->radius;
    float t0, t1;
    if(!solve_quadratic(A, B, C, t0, t1)) {
      return false;
    }
    if(t1 < t0) {
      std::swap(t0, t1);
    }

    float t_hit;
    if(ray.t_min < t0 && t0 < ray.t_max) {
      t_hit = t0;
    } else if(ray.t_min < t1 && t1 < ray.t_min) {
      t_hit = t1;
    } else {
      return false;
    }

    Point p_hit = ray.point_t(t_hit);
    float x = p_hit.v.x, y = p_hit.v.y, z = p_hit.v.z;
    float phi = atan(y, z);
    if(phi < 0.f) {
      phi = phi + TWO_PI;
    }
    float theta = acos(z * this->inv_radius);

    float r_sin_theta = sqrt(x*x + y*y);
    float inv_r_sin_theta = 1.f / r_sin_theta;

    out_diff_geom.p = p_hit;
    out_diff_geom.nn = Normal(p_hit - Point(0.f, 0.f, 0.f)) * this->inv_radius;
    out_diff_geom.u = phi * INV_TWO_PI;
    out_diff_geom.v = theta * INV_PI;
    out_diff_geom.dpdu = Vector(-TWO_PI * y, TWO_PI * x, 0.f);
    out_diff_geom.dpdv = Vector(
        PI * z * x * inv_r_sin_theta,
        PI * z * y * inv_r_sin_theta,
        -r_sin_theta);
    out_t_hit = t_hit;
    out_ray_epsilon = 5e-4f * abs(t_hit);
    return true;
  }

  bool Sphere::hit_p(const Ray& ray) const
  {
    float A = dot(ray.dir.v, ray.dir.v);
    float B = 2.f * dot(ray.orig.v, ray.dir.v);
    float C = dot(ray.orig.v, ray.orig.v) - this->radius * this->radius;
    float t0, t1;
    if(!solve_quadratic(A, B, C, t0, t1)) {
      return false;
    }

    if(ray.t_min < t0 && t0 < ray.t_max) {
      return true;
    } else if(ray.t_min < t1 && t1 < ray.t_max) {
      return true;
    } else {
      return false;
    }
  }

  Box Sphere::bound() const
  {
    float r = this->radius;
    return Box(Point(-r, -r, -r), Point(r, r, r));
  }
}

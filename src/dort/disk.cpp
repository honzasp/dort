#include "dort/disk.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  bool Disk::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    if(ray.dir.v.z == 0.f) {
      return false;
    }

    float t_hit = (this->height - ray.orig.v.z) / ray.dir.v.z;
    if(!(ray.t_min <= t_hit && t_hit <= ray.t_max)) {
      return false;
    }

    Point p = ray.point_t(t_hit);
    float dist_square = square(p.v.x) + square(p.v.y);
    if(dist_square > square(this->radius)) {
      return false;
    }

    float dist = sqrt(dist_square);
    float theta = atan(p.v.y, p.v.x);
    float inv_v = this->radius / dist;

    out_t_hit = t_hit;
    out_ray_epsilon = t_hit * 1e-3;
    out_diff_geom.p = p;
    out_diff_geom.nn = Normal(0.f, 0.f, 1.f);
    out_diff_geom.u = theta * INV_TWO_PI;
    out_diff_geom.v = dist * this->inv_radius;
    out_diff_geom.dpdu = Vector(-TWO_PI * p.v.y, TWO_PI * p.v.x, 0.f);
    out_diff_geom.dpdv = Vector(p.v.x * inv_v, p.v.y * inv_v, 0.f);
    return true;
  }

  bool Disk::hit_p(const Ray& ray) const {
    if(ray.dir.v.z == 0.f) {
      return false;
    }

    float t_hit = (this->height - ray.orig.v.z) / ray.dir.v.z;
    if(!(ray.t_min <= t_hit && t_hit <= ray.t_max)) {
      return false;
    }

    Point p = ray.point_t(t_hit);
    float dist_square = square(p.v.x) + square(p.v.y);
    return dist_square <= square(this->radius);
  }

  Box Disk::bounds() const {
    return Box(
        Point(-this->radius, -this->radius, this->height),
        Point(this->radius, this->radius, this->height));
  }

  float Disk::area() const {
    return PI * square(this->radius);
  }

  Point Disk::sample_point(float u1, float u2, Normal& out_n) const {
    Vector w = uniform_disk_sample(u1, u2);
    out_n = Normal(0.f, 0.f, 1.f);
    return Point(w.v.x * this->radius, w.v.y * this->radius, this->height);
  }

  float Disk::point_pdf(const Point&) const {
    return INV_PI * square(this->inv_radius);
  }
}

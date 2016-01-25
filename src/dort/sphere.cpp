#include <utility>
#include "dort/sphere.hpp"

namespace dort {
  Sphere::Sphere(float radius):
    radius(radius)
  { }

  bool Sphere::hit(const Ray& ray, Hit& out_hit) const
  {
    // solve a quadratic equation for t0 and t1
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

    // find the first t that is in the range
    float t_hit;
    if(t0 > ray.t_max) {
      return false;
    } else if(t1 < ray.t_min) {
      return false;
    } else if(t0 > ray.t_min) {
      t_hit = t0;
    } else {
      t_hit = t1;
    }

    // compute the hit
    out_hit.p = ray(t_hit);
    out_hit.t = t_hit;
    out_hit.normal = out_hit.p - Point(0.f, 0.f, 0.f);
    out_hit.ray_epsilon = 5e-4f * t_hit;
    return true;
  }
}

#include <utility>
#include "dort/monte_carlo.hpp"
#include "dort/sphere.hpp"

namespace dort {
  Sphere::Sphere(float radius):
    radius(radius), inv_radius(1.f / radius)
  { }

  bool Sphere::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    float t0, t1;
    if(!this->solve_hit_t(ray, t0, t1)) {
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
    out_ray_epsilon = 5e-3f * abs(t_hit);
    return true;
  }

  bool Sphere::hit_p(const Ray& ray) const {
    float t0, t1;
    if(!this->solve_hit_t(ray, t0, t1)) {
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

  Box Sphere::bounds() const {
    float r = this->radius;
    return Box(Point(-r, -r, -r), Point(r, r, r));
  }

  float Sphere::area() const {
    return FOUR_PI * this->radius * this->radius;
  }

  Point Sphere::sample_point(float u1, float u2, Normal& out_n) const {
    Vector w = uniform_sphere_sample(u1, u2);
    out_n = Normal(w);
    return Point() + w * this->radius;
  }

  float Sphere::point_pdf(const Point&) const {
    return uniform_sphere_pdf();
  }

  Point Sphere::sample_point_eye(const Point& eye,
      float u1, float u2, Normal& out_n) const
  {
    float dist_squared = length_squared(eye.v);
    if(dist_squared - this->radius * this->radius < 1e-3) {
      return this->sample_point(u1, u2, out_n);
    }

    float dist = sqrt(dist_squared);
    float inv_dist = 1.f / dist;
    float cos_theta_max = sqrt(dist_squared - this->radius * this->radius) * inv_dist;

    Vector cone_vec = uniform_cone_sample(cos_theta_max, u1, u2);
    Vector cone_z = -Vector(eye.v) * inv_dist;
    Vector cone_x, cone_y;
    coordinate_system(cone_z, cone_x, cone_y);

    Vector ray_dir = cone_x * cone_vec.v.x +
      cone_y * cone_vec.v.y + cone_z * cone_vec.v.z;
    Ray ray(eye, ray_dir);
    float t0, t1;
    if(!this->solve_hit_t(ray, t0, t1)) {
      t0 = dist;
    } else if(t1 < t0) {
      t0 = t1;
    }

    Point pt = ray.point_t(t0);
    out_n = Normal(pt.v) * this->inv_radius;
    return pt;
  }

  float Sphere::point_eye_pdf(const Point& eye, const Vector& w) const {
    float dist_squared = length_squared(eye.v);
    if(dist_squared - this->radius * this->radius < 1e-3) {
      return Shape::point_eye_pdf(eye, w);
    }
    float dist = sqrt(dist_squared);
    float inv_dist = 1.f / dist;
    float cos_theta_max = sqrt(dist_squared - this->radius * this->radius) * inv_dist;
    return uniform_cone_pdf(cos_theta_max);
  }

  bool Sphere::solve_hit_t(const Ray& ray, float& t0, float& t1) const {
    float A = dot(ray.dir.v, ray.dir.v);
    float B = 2.f * dot(ray.orig.v, ray.dir.v);
    float C = dot(ray.orig.v, ray.orig.v) - this->radius * this->radius;
    return solve_quadratic(A, B, C, t0, t1);
  }
}

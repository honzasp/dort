#include <utility>
#include "dort/monte_carlo.hpp"
#include "dort/sphere_shape.hpp"

namespace dort {
  SphereShape::SphereShape(float radius):
    radius(radius)
  { }

  bool SphereShape::hit(const Ray& ray, float& out_t_hit,
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

    float phi = atan(y, z);
    if(phi < 0.f) {
      phi = phi + TWO_PI;
    }
    float theta = acos(z / this->radius);

    float r_sin_theta = sqrt(x*x + y*y);
    float inv_r_sin_theta = 1.f / r_sin_theta;

    out_diff_geom.p = p_hit;
    out_diff_geom.nn = Normal(x, y, z) / this->radius;
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

  bool SphereShape::hit_p(const Ray& ray) const {
    float t_hit;
    return this->solve_hit_t(ray, t_hit);
  }

  Box SphereShape::bounds() const {
    float r = this->radius;
    return Box(Point(-r, -r, -r), Point(r, r, r));
  }

  float SphereShape::area() const {
    return FOUR_PI * this->radius * this->radius;
  }

  Point SphereShape::sample_point(float u1, float u2,
      Normal& out_n, float& out_ray_epsilon) const 
  {
    Vec3 w = uniform_sphere_sample(u1, u2);
    out_n = Normal(w);
    out_ray_epsilon = 5e-3f * this->radius;
    return Point() + Vector(w * this->radius);
  }

  float SphereShape::point_pdf(const Point&) const {
    return uniform_sphere_pdf() / this->area();
  }

  Point SphereShape::sample_point_eye(const Point& eye,
      float u1, float u2, Normal& out_n) const
  {
    float dist_squared = length_squared(eye.v);
    if(dist_squared - this->radius * this->radius < 1e-3) {
      float ray_epsilon;
      return this->sample_point(u1, u2, out_n, ray_epsilon);
    }

    float dist = sqrt(dist_squared);
    float inv_dist = 1.f / dist;
    float cos_theta_max = sqrt(dist_squared - this->radius * this->radius) * inv_dist;

    Vec3 cone_vec = uniform_cone_sample(cos_theta_max, u1, u2);
    Vector cone_z = -Vector(eye.v) * inv_dist;
    Vector cone_x, cone_y;
    coordinate_system(cone_z, cone_x, cone_y);

    Vector ray_dir = cone_x * cone_vec.x + cone_y * cone_vec.y + cone_z * cone_vec.z;
    Ray ray(eye, ray_dir);
    float t_hit;
    if(!this->solve_hit_t(ray, t_hit)) {
      t_hit = dist;
    }

    Point pt = ray.point_t(t_hit);
    out_n = Normal(pt.v) / this->radius;
    return pt;
  }

  float SphereShape::point_eye_pdf(const Point& eye, const Vector& w) const {
    float dist_squared = length_squared(eye.v);
    if(dist_squared - this->radius * this->radius < 1e-3) {
      return Shape::point_eye_pdf(eye, w);
    }
    float dist = sqrt(dist_squared);
    float inv_dist = 1.f / dist;
    float cos_theta_max = sqrt(dist_squared - this->radius * this->radius) * inv_dist;
    return uniform_cone_pdf(cos_theta_max);
  }

  bool SphereShape::solve_hit_t(const Ray& ray, float& out_t_hit) const {
    float a = dot(ray.dir.v, ray.dir.v);
    float b = 2.f * dot(ray.orig.v, ray.dir.v);
    float c = dot(ray.orig.v, ray.orig.v) - this->radius * this->radius;
    float t0, t1;
    if(!solve_quadratic(a, b, c, t0, t1)) {
      return false;
    }

    if(t1 < t0) {
      std::swap(t0, t1);
    }

    if(ray.t_min <= t0 && t0 <= ray.t_max) {
      out_t_hit = t0;
    } else if(ray.t_min <= t1 && t1 <= ray.t_max) {
      out_t_hit = t1;
    } else {
      return false;
    }
    return true;
  }
}

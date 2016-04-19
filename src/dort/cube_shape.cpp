#include "dort/cube_shape.hpp"

namespace dort {
  bool CubeShape::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    float t0 = ray.t_min;
    float t1 = ray.t_max;
    uint32_t hit_axis = -1u;
    bool hit_negative = false;

    for(uint32_t axis = 0; axis < 3; ++axis) {
      float inv_dir = 1.f / ray.dir.v[axis];
      float t_near = (-1.f - ray.orig.v[axis]) * inv_dir;
      float t_far = (1.f - ray.orig.v[axis]) * inv_dir;
      bool negative = false;
      if(t_near > t_far) {
        std::swap(t_near, t_far);
        negative = true;
      }

      if(t_near > t0) {
        hit_axis = axis;
        hit_negative = negative;
        t0 = t_near;
      }
      if(t_far < t1) {
        t1 = t_far;
      }
      if(t0 > t1) {
        return false;
      }
    }

    if(hit_axis == -1u) {
      return false;
    }

    out_t_hit = t0;
    out_ray_epsilon = 1e-3;
    Point p = out_diff_geom.p = ray.point_t(t0);
    out_diff_geom.nn = Normal(Vec3::axis(hit_axis, hit_negative ? -1.f : 1.f));

    if(hit_axis == 0) {
      out_diff_geom.u = p.v.y;
      out_diff_geom.v = p.v.z;
      out_diff_geom.dpdu = Vector(0.f, 1.f, 0.f);
      out_diff_geom.dpdv = Vector(0.f, 0.f, 1.f);
    } else if(hit_axis == 1) {
      out_diff_geom.u = p.v.x;
      out_diff_geom.v = p.v.z;
      out_diff_geom.dpdu = Vector(1.f, 0.f, 0.f);
      out_diff_geom.dpdv = Vector(0.f, 0.f, 1.f);
    } else if(hit_axis == 2) {
      out_diff_geom.u = p.v.x;
      out_diff_geom.v = p.v.y;
      out_diff_geom.dpdu = Vector(1.f, 0.f, 0.f);
      out_diff_geom.dpdv = Vector(0.f, 1.f, 0.f);
    }

    return true;
  }

  bool CubeShape::hit_p(const Ray& ray) const {
    float t0 = ray.t_min;
    float t1 = ray.t_max;
    for(uint32_t axis = 0; axis < 3; ++axis) {
      float inv_dir = 1.f / ray.dir.v[axis];
      float t_near = (-1.f - ray.orig.v[axis]) * inv_dir;
      float t_far = (1.f - ray.orig.v[axis]) * inv_dir;
      if(t_near > t_far) {
        std::swap(t_near, t_far);
      }

      t0 = t_near > t0 ? t_near : t0;
      t1 = t_far < t1 ? t_far : t1;
      if(t0 > t1) {
        return false;
      }
    }
    return true;
  }

  Box CubeShape::bounds() const {
    return Box(Point(-1.f, -1.f, -1.f), Point(1.f, 1.f, 1.f));
  }

  float CubeShape::area() const {
    return 24.f;
  }

  Point CubeShape::sample_point(float u1, float u2, Normal& out_n) const {
    float fface;
    float x = modf(6.f * u1, &fface);
    float y = u2;

    int32_t face = floor_int32(fface);
    int32_t axis = face / 2;
    bool negative = face % 2 != 0;
    out_n = Normal(permute(Vec3(negative ? -1.f : 1.f, 0.f, 0.f), axis));
    return Point(permute(Vec3(negative ? -1.f : 1.f, x, y), axis));
  }

  float CubeShape::point_pdf(const Point&) const {
    return 1.f/24.f;
  }

  Point CubeShape::sample_point_eye(const Point& eye,
      float u1, float u2, Normal& out_n) const 
  {
    float faxis;
    float x = modf(3.f * u1, &faxis);
    float y = u2;

    int32_t axis = floor_int32(faxis);
    bool negative = eye.v[axis] < 0.f;
    out_n = Normal(permute(Vec3(negative ? -1.f : 1.f, 0.f, 0.f), axis));
    return Point(permute(Vec3(negative ? -1.f : 1.f, x, y), axis));
  }

  float CubeShape::point_eye_pdf(const Point& eye, const Vector& w) const {
    return Shape::point_eye_pdf(eye, w);
  }
}

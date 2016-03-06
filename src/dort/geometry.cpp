#include <utility>
#include "dort/geometry.hpp"

namespace dort {
  void coordinate_system(const Vector& vec0, Vector& vec1, Vector& vec2) {
    if(abs(vec0.v.x) > abs(vec0.v.y)) {
      vec1 = Vector(-vec0.v.z, 0.f, vec0.v.x);
    } else {
      vec1 = Vector(0.f, -vec0.v.z, vec0.v.y);
    }
    vec2 = cross(vec0, vec1);
  }

  Box union_box(const Box& b1, const Box& b2) {
    return Box(
        Point(
          min(b1.p_min.v.x, b2.p_min.v.x),
          min(b1.p_min.v.y, b2.p_min.v.y),
          min(b1.p_min.v.z, b2.p_min.v.z)),
        Point(
          max(b1.p_max.v.x, b2.p_max.v.x),
          max(b1.p_max.v.y, b2.p_max.v.y),
          max(b1.p_max.v.z, b2.p_max.v.z)));
  }

  Box union_box(const Box& box, const Point& pt) {
    return Box(
        Point(
          min(box.p_min.v.x, pt.v.x),
          min(box.p_min.v.y, pt.v.y),
          min(box.p_min.v.z, pt.v.z)),
        Point(
          max(box.p_max.v.x, pt.v.x),
          max(box.p_max.v.y, pt.v.y),
          max(box.p_max.v.z, pt.v.z)));
  }

  bool Box::hit(const Ray& ray, float& out_t) const {
    float t0 = ray.t_min;
    float t1 = ray.t_max;
    for(uint32_t i = 0; i < 3; ++i) {
      float inv_dir = 1.f / ray.dir.v[i];
      float t_near = (this->p_min.v[i] - ray.orig.v[i]) * inv_dir;
      float t_far = (this->p_max.v[i] - ray.orig.v[i]) * inv_dir;
      if(t_near > t_far) {
        std::swap(t_near, t_far);
      }
      // if inv_dir is infinity (division by zero), t_near and t_far are NaN's,
      // so these comparisons must ensure that t0 and t1 are not updated in that
      // case
      t0 = t_near > t0 ? t_near : t0;
      t1 = t_far < t1 ? t_far : t1;
      if(t0 > t1) {
        return false;
      }
    }
    out_t = t0;
    return true;
  }

  bool Box::hit_p(const Ray& ray) const {
    float unused_t;
    return this->hit(ray, unused_t);
  }

  bool Box::fast_hit_p(const Ray& ray,
      const Vector& inv_dir, bool dir_is_neg[3]) const
  {
    float t_min =  ((*this)[  dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;
    float t_max =  ((*this)[1-dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;

    float ty_min = ((*this)[  dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    float ty_max = ((*this)[1-dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    if(t_min > ty_max || ty_min > t_max) {
      return false;
    }

    if(ty_min > t_min) {
      t_min = ty_min;
    }
    if(ty_max < t_max) {
      t_max = ty_max;
    }

    float tz_min = ((*this)[  dir_is_neg[2]].v.z - ray.orig.v.z) * inv_dir.v.z;
    float tz_max = ((*this)[1-dir_is_neg[2]].v.z - ray.orig.v.z) * inv_dir.v.z;
    if(t_min > tz_max || t_max < tz_min) {
      return false;
    }

    if(tz_min > t_min) {
      t_min = tz_min;
    }
    if(t_max > tz_max) {
      t_max = tz_max;
    }

    return (t_min < ray.t_max) && (t_max > ray.t_min);
  }
}

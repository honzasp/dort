#pragma once
#include "dort/geometry.hpp"

namespace dort {
  struct Box {
    union {
      struct {
        Point p_min;
        Point p_max;
      };
      Point p[2];
    };

    Box(): 
      p_min(INFINITY, INFINITY, INFINITY),
      p_max(-INFINITY, -INFINITY, -INFINITY) { }
    Box(const Box& box):
      p_min(box.p_min), p_max(box.p_max) { }
    Box(const Point& p_min, const Point& p_max):
      p_min(p_min), p_max(p_max) { }
    Box& operator=(const Box& box) {
      this->p_min = box.p_min;
      this->p_max = box.p_max;
      return *this;
    }

    const Point& operator[](uint32_t i) const {
      assert(i <= 1);
      return this->p[i];
    }

    float area() const {
      Vec3 r = (this->p_max - this->p_min).v;
      return 2.f * (r.y * r.z + r.x * r.z + r.y * r.z);
    }

    Point centroid() const {
      return (this->p_max + this->p_min) * 0.5f;
    }

    uint8_t max_axis() const {
      return (this->p_max - this->p_min).v.max_axis();
    }

    bool contains(const Point& pt) const {
      return 
        this->p_min.v.x <= pt.v.x && pt.v.x <= this->p_max.v.x &&
        this->p_min.v.y <= pt.v.y && pt.v.y <= this->p_max.v.y &&
        this->p_min.v.z <= pt.v.z && pt.v.z <= this->p_max.v.z;
    }

    bool hit(const Ray& ray, float& out_t) const;
    bool hit_p(const Ray& ray) const;
    bool fast_hit_p(const Ray& ray,
        const Vector& inv_dir, bool dir_is_neg[3]) const;
  };

  Box union_box(const Box& b1, const Box& b2);
  Box union_box(const Box& box, const Point& pt);

  inline bool is_finite(const Box& box) {
    return is_finite(box.p_min) && is_finite(box.p_max);
  }
}

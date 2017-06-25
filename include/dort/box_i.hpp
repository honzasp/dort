#pragma once
#include "dort/vec_3i.hpp"

namespace dort {
  struct Boxi {
    Vec3i p_min;
    Vec3i p_max;

    Boxi():
      p_min(INT32_MAX, INT32_MAX, INT32_MAX),
      p_max(INT32_MIN, INT32_MIN, INT32_MIN) { }
    Boxi(const Vec3i& p_min, const Vec3i& p_max):
      p_min(p_min), p_max(p_max) { }
    Boxi(const Boxi& box): p_min(box.p_min), p_max(box.p_max) { }
    Boxi& operator=(const Boxi& box) {
      this->p_min = box.p_min;
      this->p_max = box.p_max;
      return *this;
    }

    const Vec3i& operator[](uint32_t i) const {
      assert(i <= 1);
      return i == 0 ? this->p_min : this->p_max;
    }

    uint8_t max_axis() const {
      return (this->p_max - this->p_min).max_axis();
    }

    std::tuple<Boxi, Boxi, int32_t> split(uint8_t axis) const {
      int32_t mid = (this->p_min[axis] + this->p_max[axis]) / 2;
      Vec3i left_max = this->p_max;
      left_max[axis] = mid;
      Vec3i right_min = this->p_min;
      right_min[axis] = mid;
      return std::make_tuple(Boxi(this->p_min, left_max),
          Boxi(right_min, this->p_max), mid);
    }

    bool contains(const Vec3i& v) const {
      return v.x >= this->p_min.x && v.y >= this->p_min.y && v.z >= this->p_min.z
        && v.x < this->p_max.x && v.y < this->p_max.y && v.z < this->p_max.z;
    }
  };

  inline Boxi operator+(const Boxi& box, const Vec3i& vec) {
    return Boxi(box.p_min + vec, box.p_max + vec);
  }
}

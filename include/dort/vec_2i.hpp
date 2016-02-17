#pragma once
#include "dort/dort.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  struct Vec2i {
    union {
      struct {
        int32_t x;
        int32_t y;
      };
      int32_t coords[2];
    };

    Vec2i(): Vec2i(0, 0) { }
    Vec2i(int32_t x, int32_t y) {
      this->x = x;
      this->y = y;
    }
    Vec2i(const Vec2i& v) {
      this->x = v.x;
      this->y = v.y;
    }

    int32_t operator[](uint32_t i) const {
      assert(i < 2);
      return this->coords[i];
    }
    int32_t& operator[](uint32_t i) {
      assert(i < 2);
      return this->coords[i];
    }
  };

  inline Vec2i operator+(const Vec2i& v1, const Vec2i& v2) {
    return Vec2i(v1.x + v2.x, v1.y + v2.y);
  }
  inline Vec2i operator-(const Vec2i& v1, const Vec2i& v2) {
    return Vec2i(v1.x - v2.x, v1.y - v2.y);
  }
  inline Vec2i operator-(const Vec2i& v) {
    return Vec2i(-v.x, -v.y);
  }
  inline Vec2i operator*(const Vec2i& v, int32_t a) {
    return Vec2i(v.x * a, v.y * a);
  }
  inline Vec2i operator*(int32_t a, const Vec2i& v) {
    return Vec2i(a * v.x, a * v.y);
  }

  inline Vec2i floor_vec2i(const Vec2& v) {
    return Vec2i(floor_int32(v.x), floor_int32(v.y));
  }
  inline Vec2i ceil_vec2i(const Vec2& v) {
    return Vec2i(ceil_int32(v.x), ceil_int32(v.y));
  }
}

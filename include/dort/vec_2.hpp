#pragma once
#include "dort/math.hpp"

namespace dort {
  struct Vec2 {
    union {
      struct {
        float x;
        float y;
      };
      float coords[2];
    };

    Vec2(): Vec2(0.f, 0.f) { }
    Vec2(float x, float y) {
      this->x = x;
      this->y = y;
    }
    Vec2(const Vec2& v) {
      this->x = v.x;
      this->y = v.y;
    }

    float operator[](uint32_t i) const {
      assert(i < 2);
      return this->coords[i];
    }

    float& operator[](uint32_t i) {
      assert(i < 2);
      return this->coords[i];
    }
  };

  inline Vec2 operator+(Vec2 v1, Vec2 v2) {
    return Vec2(v1.x + v2.x, v1.y + v2.y);
  }
  inline Vec2 operator-(Vec2 v1, Vec2 v2) {
    return Vec2(v1.x - v2.x, v1.y - v2.y);
  }
  inline Vec2 operator-(Vec2 v) {
    return Vec2(-v.x, -v.y);
  }
  inline Vec2 operator*(Vec2 v, float a) {
    return Vec2(v.x * a, v.y * a);
  }
  inline Vec2 operator*(float a, Vec2 v) {
    return Vec2(a * v.x, a * v.y);
  }
  inline Vec2 operator/(Vec2 v, float a) {
    return v * (1.f / a);
  }
  inline Vec2 operator/(float a, Vec2 v) {
    return Vec2(a / v.x, a / v.y);
  }
  inline Vec2 operator*(Vec2 v1, Vec2 v2) {
    return Vec2(v1.x * v2.x, v1.y * v2.y);
  }
  inline Vec2 operator/(Vec2 v1, Vec2 v2) {
    return Vec2(v1.x / v2.x, v1.y / v2.y);
  }

  inline bool operator==(Vec2 v1, Vec2 v2) {
    return v1.x == v2.x && v1.y == v2.y;
  }
  inline bool operator!=(Vec2 v1, Vec2 v2) {
    return !(v1 == v2);
  }

  inline float dot(Vec2 v1, Vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
  }
  inline float length_squared(Vec2 v) {
    return dot(v, v);
  }
  inline float length(Vec2 v) {
    return sqrt(length_squared(v));
  }

  inline Vec2 abs(Vec2 v) {
    return Vec2(abs(v.x), abs(v.y));
  }

  inline bool is_finite(Vec2 v) {
    return is_finite(v.x) && is_finite(v.y);
  }
  inline bool is_nonnegative(Vec2 v) {
    return v.x >= 0.f && v.y >= 0.f;
  }
  inline Vec2 floor(Vec2 v) {
    return Vec2(floor(v.x), floor(v.y));
  }
}

#pragma once
#include "dort/math.hpp"

namespace dort {
  struct Vec3 {
    union {
      struct {
        float x;
        float y;
        float z;
      };
      float coords[3];
    };

    Vec3(): Vec3(0.f, 0.f, 0.f) { }
    Vec3(float x, float y, float z) {
      this->x = x;
      this->y = y;
      this->z = z;
    }
    Vec3(const Vec3& v) {
      this->x = v.x;
      this->y = v.y;
      this->z = v.z;
    }

    static Vec3 axis(int32_t axis, float dir = 1.f) {
      assert(axis >= 0 && axis < 3);
      if(axis == 0) {
        return Vec3(dir, 0.f, 0.f);
      } else if(axis == 1) {
        return Vec3(0.f, dir, 0.f);
      } else {
        return Vec3(0.f, 0.f, dir);
      }
    }

    float operator[](uint32_t i) const {
      assert(i < 3);
      return this->coords[i];
    }

    float& operator[](uint32_t i) {
      assert(i < 3);
      return this->coords[i];
    }

    uint8_t max_axis() const {
      return arg_max_3(this->x, this->y, this->z);
    }
  };

  inline Vec3 operator+(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }
  inline Vec3 operator-(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
  }
  inline Vec3 operator-(const Vec3& v) {
    return Vec3(-v.x, -v.y, -v.z);
  }
  inline Vec3 operator*(const Vec3& v, float a) {
    return Vec3(v.x * a, v.y * a, v.z * a);
  }
  inline Vec3 operator*(float a, const Vec3& v) {
    return Vec3(a * v.x, a * v.y, a * v.z);
  }
  inline Vec3 operator/(const Vec3& v, float a) {
    return v * (1.f / a);
  }
  inline Vec3 operator/(float a, const Vec3& v) {
    return Vec3(a / v.x, a / v.y, a / v.z);
  }
  inline Vec3 operator*(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
  }
  inline Vec3 operator/(const Vec3& v1, const Vec3& v2) {
    return Vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
  }

  inline bool operator==(const Vec3& v1, const Vec3& v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
  }
  inline bool operator!=(const Vec3& v1, const Vec3& v2) {
    return !(v1 == v2);
  }

  inline float dot(const Vec3& v1, const Vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }
  inline Vec3 cross(const Vec3& v1, const Vec3& v2) {
    return Vec3(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
  }

  inline float length_squared(const Vec3& v) {
    return dot(v, v);
  }
  inline float length(const Vec3& v) {
    return sqrt(length_squared(v));
  }
  inline Vec3 normalize(const Vec3& v) {
    return v / length(v);
  }

  inline bool is_finite(const Vec3& v) {
    return is_finite(v.x) && is_finite(v.y) && is_finite(v.z);
  }
  inline bool is_nonnegative(const Vec3& v) {
    return v.x >= 0.f && v.y >= 0.f && v.z >= 0.f;
  }

  inline Vec3 abs(const Vec3& v) {
    return Vec3(abs(v.x), abs(v.y), abs(v.z));
  }
  inline Vec3 floor(const Vec3& v) {
    return Vec3(floor(v.x), floor(v.y), floor(v.z));
  }
  inline Vec3 sqrt(const Vec3& v) {
    return Vec3(sqrt(v.x), sqrt(v.y), sqrt(v.z));
  }

  inline Vec3 permute(const Vec3& v, int32_t rot) {
    assert(rot >= 0 && rot < 3);
    if(rot == 1) {
      return Vec3(v.y, v.z, v.x);
    } else if(rot == 2) {
      return Vec3(v.z, v.x, v.y);
    } else {
      return v;
    }
  }
}

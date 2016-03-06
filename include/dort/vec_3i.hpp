#pragma once
#include "dort/math.hpp"
#include "dort/vec_3.hpp"

namespace dort {
  struct Vec3i {
    union {
      struct {
        int32_t x;
        int32_t y;
        int32_t z;
      };
      int32_t coords[3];
    };

    Vec3i(): Vec3i(0, 0, 0) { }
    Vec3i(int32_t x, int32_t y, int32_t z) {
      this->x = x;
      this->y = y;
      this->z = z;
    }
    Vec3i(const Vec3i& v) {
      this->x = v.x;
      this->y = v.y;
      this->z = v.z;
    }

    int32_t operator[](uint32_t i) const {
      assert(i < 3);
      return this->coords[i];
    }

    int32_t& operator[](uint32_t i) {
      assert(i < 3);
      return this->coords[i];
    }

    uint8_t max_axis() const {
      return arg_max_3(this->x, this->y, this->z);
    }

    explicit operator Vec3() const {
      return Vec3(float(this->x), float(this->y), float(this->z));
    }
  };

  inline Vec3i operator+(const Vec3i& v1, const Vec3i& v2) {
    return Vec3i(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }
  inline Vec3i operator-(const Vec3i& v1, const Vec3i& v2) {
    return Vec3i(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
  }
  inline Vec3i operator-(const Vec3i& v) {
    return Vec3i(-v.x, -v.y, -v.z);
  }
  inline Vec3i operator*(const Vec3i& v1, const Vec3i& v2) {
    return Vec3i(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
  }
  inline Vec3i operator*(int32_t a, const Vec3i& v2) {
    return Vec3i(a * v2.x, a * v2.y, a * v2.z);
  }

  inline bool operator==(const Vec3i& v1, const Vec3i& v2) {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
  }
  inline bool operator!=(const Vec3i& v1, const Vec3i& v2) {
    return !(v1 == v2);
  }

  inline int32_t dot(const Vec3i& v1, const Vec3i& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  inline Vec3i floor_vec3i(const Vec3& v) {
    return Vec3i(floor_int32(v.x), floor_int32(v.y), floor_int32(v.z));
  }
  inline Vec3i ceil_vec3i(const Vec3& v) {
    return Vec3i(ceil_int32(v.x), ceil_int32(v.y), ceil_int32(v.z));
  }
}

namespace std {
  template<> struct hash<dort::Vec3i> {
    using argument_type = dort::Vec3i;
    using result_type = std::size_t;

    std::size_t operator()(const dort::Vec3i& v) const noexcept {
      int32_t h0 = v.x;
      int32_t h1 = v.y + 0x9e3779b9 + (h0 << 6) + (h0 >> 2);
      int32_t h2 = v.z + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
      return h2;
    }
  };
}

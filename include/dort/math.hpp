#pragma once
#include <cmath>
#include <cstdint>
#include <limits>
#include "dort/dort.hpp"

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef M_PI
#undef M_PI
#endif

namespace dort {
  constexpr float INFINITY = std::numeric_limits<float>::infinity();
  constexpr float SIGNALING_NAN = std::numeric_limits<float>::signaling_NaN();
  constexpr float QUIET_NAN = std::numeric_limits<float>::quiet_NaN();
  constexpr float PI = 3.141592653589793f;
  constexpr float TWO_PI = 6.283185307179586f;
  constexpr float FOUR_PI = 12.56637061435917f;
  constexpr float INV_PI = 0.318309886183790f;
  constexpr float INV_TWO_PI = 0.159154943091895f;
  constexpr float INV_FOUR_PI = 0.079577471545947f;

  template<class T>
  T clamp(T val, T min, T max) {
    if(val > max) {
      return max;
    } else if(val < min) {
      return min;
    } else {
      return val;
    }
  }

  inline float clamp(float val) {
    return clamp(val, 0.f, 1.f);
  }

  template<class T>
  T lerp(float t, T x0, T x1) {
    return t * x0 + (1.f - t) * x1;
  }

  inline float sqrt(float a) { return std::sqrt(a); }
  inline float exp(float a) { return std::exp(a); }
  inline float pow(float b, float e) { return std::pow(b, e); }
  inline float floor(float a) { return std::floor(a); }
  inline float ceil(float a) { return std::ceil(a); }
  inline float abs(float a) { return std::abs(a); }
  inline float sin(float a) { return std::sin(a); }
  inline float cos(float a) { return std::cos(a); }
  inline float tan(float a) { return std::tan(a); }
  inline float asin(float y) { return std::asin(y); }
  inline float acos(float x) { return std::acos(x); }
  inline float atan(float y, float x) { return std::atan2(y, x); }

  inline bool is_finite(float a) { return std::isfinite(a); }
  template<class T> T min(T a, T b) { return (b < a) ? b : a; }
  template<class T> T max(T a, T b) { return (a < b) ? b : a; }

  inline int32_t floor_int32(float a) { return int32_t(floor(a)); }
  inline int32_t ceil_int32(float a) { return int32_t(ceil(a)); }
  inline float mul_power_of_two(float a, int32_t exp) { return std::ldexp(a, exp); }
  inline float square(float a) { return a * a; }
  inline float cube(float a) { return a * a * a; }

  inline float sinc(float x) {
    float t = abs(x) * PI;
    return t < 1e-5 ? 1.f : sin(t) / t;
  }

  inline uint32_t round_up_power_of_two(uint32_t x) {
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);
    return x + 1;
  }

  bool solve_quadratic(float A, float B, float C, float& out_x1, float& out_x2);
}

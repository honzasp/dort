#pragma once
#include <cmath>
#include <cstdint>
#include <limits>

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef M_PI
#undef M_PI
#endif

namespace dort {
  constexpr float INFINITY = std::numeric_limits<float>::infinity();
  constexpr float PI = 3.141592653589793;
  constexpr float INV_PI = 0.3183098861837907;

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

  template<class T>
  T lerp(float t, T x0, T x1) {
    return t * x0 + (1.f - t) * x1;
  }

  inline float sqrt(float a) { return std::sqrt(a); }
  inline float floor(float a) { return std::floor(a); }
  inline float abs(float a) { return std::abs(a); }
  inline bool is_finite(float a) { return std::isfinite(a); }

  inline int32_t floor_int32(float a) {
    return int32_t(floor(a));
  }

  bool solve_quadratic(float A, float B, float C, float& out_x1, float& out_x2);
}

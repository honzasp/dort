#pragma once
#include "dort/vec_3.hpp"

namespace dort {
  struct RgbSpectrum {
    static constexpr uint32_t SAMPLES = 3;

    Vec3 rgb;

    RgbSpectrum(): rgb() {}
    explicit RgbSpectrum(float level): rgb(level, level, level) {}
    RgbSpectrum(float r, float g, float b): rgb(r, g, b) {}
    explicit RgbSpectrum(const Vec3& rgb): rgb(rgb) {}

    static RgbSpectrum from_rgb(float r, float g, float b) {
      return RgbSpectrum(r, g, b);
    }

    float red() const { return this->rgb.x; }
    float green() const { return this->rgb.y; }
    float blue() const { return this->rgb.z; }
    float average() const {
      return (1.f / 3.f) * (this->rgb.x + this->rgb.y + this->rgb.z); 
    }
    bool is_black() const { return this->rgb == Vec3(0.f, 0.f, 0.f); }

    float sample(uint32_t i) const { return this->rgb[i]; }
    void sample(uint32_t i, float x) { this->rgb[i] = x; }
  };

  inline RgbSpectrum operator+(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return RgbSpectrum(s1.rgb + s2.rgb);
  }
  inline RgbSpectrum operator-(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return RgbSpectrum(s1.rgb - s2.rgb);
  }
  inline RgbSpectrum operator-(const RgbSpectrum& s1) {
    return RgbSpectrum(-s1.rgb);
  }
  inline RgbSpectrum operator*(float a, const RgbSpectrum& s) {
    return RgbSpectrum(a * s.rgb);
  }
  inline RgbSpectrum operator*(const RgbSpectrum& s, float a) {
    return RgbSpectrum(s.rgb * a);
  }
  inline RgbSpectrum operator/(const RgbSpectrum& s, float a) {
    return RgbSpectrum(s.rgb / a);
  }
  inline RgbSpectrum operator/(float a, const RgbSpectrum& s) {
    return RgbSpectrum(a / s.rgb);
  }
  inline RgbSpectrum operator*(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return RgbSpectrum(s1.rgb * s2.rgb);
  }
  inline RgbSpectrum operator/(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return RgbSpectrum(s1.rgb / s2.rgb);
  }
  inline bool operator==(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return s1.rgb == s2.rgb;
  }

  inline RgbSpectrum operator+=(RgbSpectrum& s1, const RgbSpectrum& s2) {
    return s1 = s1 + s2;
  }
  inline RgbSpectrum operator-=(RgbSpectrum& s1, const RgbSpectrum& s2) {
    return s1 = s1 - s2;
  }
  inline RgbSpectrum operator*=(RgbSpectrum& s1, const RgbSpectrum& s2) {
    return s1 = s1 * s2;
  }
  inline RgbSpectrum operator/=(RgbSpectrum& s1, const RgbSpectrum& s2) {
    return s1 = s1 / s2;
  }
  inline RgbSpectrum operator*=(RgbSpectrum& s1, float a) {
    return s1 = s1 * a;
  }
  inline RgbSpectrum operator/=(RgbSpectrum& s1, float a) {
    return s1 = s1 / a;
  }

  inline bool is_finite(const RgbSpectrum& s) {
    return is_finite(s.rgb);
  }
  inline bool is_nonnegative(const RgbSpectrum& s) {
    return is_nonnegative(s.rgb);
  }
}

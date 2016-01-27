#pragma once

namespace dort {
  struct RgbSpectrum {
    Vec3 rgb;

    RgbSpectrum(): rgb() {}
    RgbSpectrum(float level): rgb(level, level, level) {}
    RgbSpectrum(float r, float g, float b): rgb(r, g, b) {}
    explicit RgbSpectrum(const Vec3& rgb): rgb(rgb) {}

    static RgbSpectrum from_rgb(float r, float g, float b) {
      return RgbSpectrum(r, g, b);
    }

    float red() const { return this->rgb.x; }
    float green() const { return this->rgb.y; }
    float blue() const { return this->rgb.z; }
  };

  inline RgbSpectrum operator+(const RgbSpectrum& s1, const RgbSpectrum& s2) {
    return RgbSpectrum(s1.rgb + s2.rgb);
  }

  inline RgbSpectrum operator*(float a, const RgbSpectrum& s) {
    return RgbSpectrum(a * s.rgb);
  }

  inline RgbSpectrum operator*(const RgbSpectrum& s, float a) {
    return RgbSpectrum(s.rgb * a);
  }

  using Spectrum = RgbSpectrum;
}

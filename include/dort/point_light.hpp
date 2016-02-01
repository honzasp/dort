#pragma once
#include "dort/light.hpp"

namespace dort {
  class PointLight: public Light {
    Point pt;
    Spectrum intensity;
  public:
    PointLight(const Point& pt, const Spectrum& intensity):
      pt(pt), intensity(intensity) {}
    virtual Spectrum sample_radiance(const Point& pt, float pt_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow) const override final;
  };
}


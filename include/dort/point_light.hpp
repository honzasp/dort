#pragma once
#include "dort/light.hpp"

namespace dort {
  class PointLight final: public Light {
    Point pt;
    Spectrum intensity;
  public:
    PointLight(const Point& pt, const Spectrum& intensity):
      pt(pt), intensity(intensity) {}
    virtual Spectrum sample_radiance(const Point& eye, float eye_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
        Rng& rng) const override final;
    virtual float radiance_pdf(const Point& eye, const Vector& wi) const override final;
    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual bool is_delta() const override final;
  };
}


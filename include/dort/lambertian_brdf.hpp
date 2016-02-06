#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  class LambertianBrdf: public Bxdf {
    Spectrum reflectance;
  public:
    LambertianBrdf(const Spectrum& reflectance):
      Bxdf(BxdfFlags(BSDF_REFLECTION | BSDF_DIFFUSE)),
      reflectance(reflectance) 
    { }

    virtual Spectrum f(const Vector&, const Vector&) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };
}


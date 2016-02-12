#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  class OrenNayarBrdf final: public Bxdf {
    Spectrum reflectance;
    float a;
    float b;
  public:
    OrenNayarBrdf(const Spectrum& reflectance, float sigma);
    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };
}

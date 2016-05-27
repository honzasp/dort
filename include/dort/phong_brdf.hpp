#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  class PhongBrdf final: public Bxdf {
    Spectrum k_diffuse;
    Spectrum k_glossy;
    float exponent;
    float glossy_pdf;
  public:
    PhongBrdf(const Spectrum& k_diffuse, const Spectrum& k_glossy, float exponent);
    virtual Spectrum f(const Vector&, const Vector&) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };
}


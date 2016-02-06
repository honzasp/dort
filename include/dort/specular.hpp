#pragma once
#include "dort/bsdf.hpp"
#include "dort/fresnel.hpp"

namespace dort {
  class SpecularBrdf: public Bxdf {
    Spectrum reflectance;
    std::unique_ptr<Fresnel> fresnel;
  public:
    SpecularBrdf(const Spectrum& reflectance, std::unique_ptr<Fresnel> fresnel):
      Bxdf(BxdfFlags(BSDF_REFLECTION | BSDF_SPECULAR)),
      reflectance(reflectance), fresnel(std::move(fresnel)) { }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };

  class SpecularBtdf: public Bxdf {
    Spectrum transmittance;
    FresnelDielectric fresnel;
  public:
    SpecularBtdf(const Spectrum& transmittance, FresnelDielectric fresnel):
      Bxdf(BxdfFlags(BSDF_TRANSMISSION | BSDF_SPECULAR)),
      transmittance(transmittance), fresnel(fresnel) { }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };
}

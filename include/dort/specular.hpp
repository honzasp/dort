#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  struct Fresnel {
    virtual float reflectance(float cos_i) const = 0;
  };

  float fresnel_conductor(float eta, float k, float cos_i);
  float fresnel_dielectric(float eta_t, float eta_i, float cos_i, float cos_t);

  struct FresnelConductor: public Fresnel {
    float eta;
    float k;
    FresnelConductor(float eta, float k):
      eta(eta), k(k) { }

    virtual float reflectance(float cos_i) const override final;
  };

  struct FresnelDielectric: public Fresnel {
    float eta_i;
    float eta_t;
    FresnelDielectric(float eta_i, float eta_t):
      eta_i(eta_i), eta_t(eta_t) { }

    virtual float reflectance(float cos_i) const override final;
  };

  struct FresnelConstant: public Fresnel {
    float reflect;
    FresnelConstant(float reflectance):
      reflect(reflectance) { }

    virtual float reflectance(float) const override final
    { return this->reflect; }
  };

  class SpecularBrdf: public Bxdf {
    Spectrum reflectance;
    std::unique_ptr<Fresnel> fresnel;
  public:
    SpecularBrdf(const Spectrum& reflectance, std::unique_ptr<Fresnel> fresnel):
      Bxdf(BxdfFlags(BSDF_REFLECTION | BSDF_SPECULAR)),
      reflectance(reflectance), fresnel(std::move(fresnel)) { }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, Rng& rng) const override final;
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
        float& out_pdf, Rng& rng) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  };
}

#pragma once
#include "dort/bsdf.hpp"
#include "dort/fresnel.hpp"

namespace dort {
  class MicrofacetDistribution {
  public:
    virtual float area_pdf(const Vector& wh) const = 0;
    virtual float dir_pdf(const Vector& wh) const = 0;
    virtual Vector sample_dir(float u1, float u2, float& out_pdf) const = 0;
  };

  class TorranceSparrowBrdf final: public Bxdf {
    Spectrum reflectance;
    std::unique_ptr<MicrofacetDistribution> microfacet_dis;
    std::unique_ptr<Fresnel> fresnel;
  public:
    TorranceSparrowBrdf(const Spectrum& reflectance,
        std::unique_ptr<MicrofacetDistribution> microfacet_dis,
        std::unique_ptr<Fresnel> fresnel):
      Bxdf(BxdfFlags(BSDF_REFLECTION | BSDF_GLOSSY)),
      reflectance(reflectance),
      microfacet_dis(std::move(microfacet_dis)),
      fresnel(std::move(fresnel))
    { }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  private:
    Spectrum f(const Vector& wo, const Vector& wh, const Vector& wi) const;
    float geometric_attenuation(const Vector& wo,
        const Vector& wh, const Vector& wi) const;
  };

  class BlinnMicrofacetDistribution final: public MicrofacetDistribution {
    float exponent;
  public:
    BlinnMicrofacetDistribution(float exponent): exponent(exponent) { }
    virtual float area_pdf(const Vector& wh) const override final;
    virtual float dir_pdf(const Vector& wh) const override final;
    virtual Vector sample_dir(float u1, float u2, float& out_pdf) const override final;
  };
}

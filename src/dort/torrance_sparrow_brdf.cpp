#include "dort/torrance_sparrow_brdf.hpp"

namespace dort {
  Spectrum TorranceSparrowBrdf::f(const Vector& wo, const Vector& wi) const {
    Vector wh = normalize(wo + wi);
    return this->f(wo, wh, wi);
  }

  Spectrum TorranceSparrowBrdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    float wh_dir_pdf;
    Vector wh = this->microfacet_dis->sample_dir(u1, u2, wh_dir_pdf);
    Vector wi = 2.f * dot(wo, wh) * wh - wo;
    out_wi = wi;
    out_pdf = wh_dir_pdf / (4.f * dot(wo, wi));

    if(!Bsdf::same_hemisphere(wo, wi)) {
      return Spectrum(0.f);
    }
    return this->f(wo, wh, wi);
  }

  float TorranceSparrowBrdf::f_pdf(const Vector& wo, const Vector& wi) const {
    Vector wh = normalize(wo + wi);
    return this->microfacet_dis->dir_pdf(wh) / (4.f * dot(wo, wi));
  }

  Spectrum TorranceSparrowBrdf::f(const Vector& wo,
      const Vector& wh, const Vector& wi) const
  {
    float c = 4.f * Bsdf::abs_cos_theta(wo) * Bsdf::abs_cos_theta(wi);
    if(c == 0.f) {
      return Spectrum(0.f);
    }

    float d = this->microfacet_dis->area_pdf(wh);
    float fr = this->fresnel->reflectance(Bsdf::abs_cos_theta(wo));
    float g = this->geometric_attenuation(wo, wh, wi);
    return this->reflectance * (d * fr * g / c);
  }

  float TorranceSparrowBrdf::geometric_attenuation(
      const Vector& wo, const Vector& wh, const Vector& wi) const
  {
    return min(1.f, 2.f * Bsdf::abs_cos_theta(wh) / dot(wo, wh) *
        min(Bsdf::abs_cos_theta(wo), Bsdf::abs_cos_theta(wi)));
  }

  float BlinnMicrofacetDistribution::area_pdf(const Vector& wh) const {
    return (this->exponent + 2.f) * INV_TWO_PI * 
      pow(Bsdf::abs_cos_theta(wh), this->exponent);
  }

  float BlinnMicrofacetDistribution::dir_pdf(const Vector& wh) const {
    return (this->exponent + 1.f) * INV_TWO_PI *
      pow(Bsdf::abs_cos_theta(wh), this->exponent);
  }

  Vector BlinnMicrofacetDistribution::sample_dir(
      float u1, float u2, float& out_pdf) const 
  {
    float phi = TWO_PI * u2;
    float cos_theta = pow(u1, 1.f / (this->exponent + 1.f));
    float sin_theta = sqrt(max(0.f, 1.f - square(cos_theta)));
    out_pdf = (this->exponent + 1.f) * pow(cos_theta, this->exponent);
    return Vector(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
  }
}

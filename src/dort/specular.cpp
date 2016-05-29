#include "dort/fresnel.hpp"
#include "dort/specular.hpp"

namespace dort {
  template<class F>
  Spectrum SpecularBrdf<F>::f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  template<class F>
  Spectrum SpecularBrdf<F>::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    (void)u1; (void)u2;
    if(Bsdf::cos_theta(wo) == 0.f) {
      out_wi = Vector();
      out_pdf = 0.f;
      return Spectrum(0.f);
    }

    Vector transmitted_wi;
    float reflect = specular_reflectance(this->fresnel, wo, transmitted_wi);
    out_wi = Vector(-wo.v.x, -wo.v.y, wo.v.z);
    out_pdf = 1.f;
    return this->reflectance * reflect / Bsdf::abs_cos_theta(wo);
  }

  template<class F>
  float SpecularBrdf<F>::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  template class SpecularBrdf<FresnelConductor>;
  template class SpecularBrdf<FresnelDielectric>;
  template class SpecularBrdf<FresnelConstant>;

  template<class F>
  Spectrum SpecularBtdf<F>::f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  template<class F>
  Spectrum SpecularBtdf<F>::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const 
  {
    (void)u1; (void)u2;
    if(Bsdf::cos_theta(wo) == 0.f) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }

    float reflect = specular_reflectance(this->fresnel, wo, out_wi);
    float tau = max(0.f, 1.f - reflect);
    out_pdf = 1.f;
    return this->transmittance * tau / Bsdf::abs_cos_theta(wo);
  }

  template<class F>
  float SpecularBtdf<F>::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  template<class F>
  float specular_reflectance(const F& fresnel, const Vector& wo,
      Vector& out_transmit_wi) 
  {
    float eta = Bsdf::cos_theta(wo) < 0.f ? fresnel.eta : fresnel.inv_eta;
    float sin_i_square = square(eta) * Bsdf::sin_theta_square(wo);
    if(sin_i_square >= 1.f) {
      out_transmit_wi = Vector();
      return 1.f;
    }

    float cos_i = -copysign(sqrt(1.f - sin_i_square), Bsdf::cos_theta(wo));
    out_transmit_wi = Vector(-wo.v.x * eta, -wo.v.y * eta, cos_i);
    return fresnel.reflectance(cos_i, Bsdf::cos_theta(wo));
  }

  template class SpecularBtdf<FresnelConductor>;
  template class SpecularBtdf<FresnelDielectric>;
  template class SpecularBtdf<FresnelConstant>;
}

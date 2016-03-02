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

    float cos_theta = Bsdf::cos_theta(wo);
    if(cos_theta == 0.f) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }

    out_wi = Vector(-wo.v.x, -wo.v.y, wo.v.z);
    out_pdf = 1.f;
    return this->reflectance * this->fresnel.reflectance(cos_theta) / abs(cos_theta);
  }

  template<class F>
  float SpecularBrdf<F>::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  template class SpecularBrdf<FresnelConductor>;
  template class SpecularBrdf<FresnelDielectric>;
  template class SpecularBrdf<FresnelConstant>;

  Spectrum SpecularBtdf::f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  Spectrum SpecularBtdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const 
  {
    (void)u1; (void)u2;

    if(Bsdf::cos_theta(wo) == 0.f) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }

    float eta;
    if(Bsdf::cos_theta(wo) > 0.f) {
      eta = this->fresnel.eta_i() / this->fresnel.eta_t();
    } else {
      eta = this->fresnel.eta_t() / this->fresnel.eta_i();
    }

    float sin_t_square = square(eta) * Bsdf::sin_theta_square(wo);
    if(sin_t_square > 1.f) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }

    float cos_t;
    if(Bsdf::cos_theta(wo) > 0.f) {
      cos_t = -sqrt(1.f - sin_t_square);
    } else {
      cos_t = sqrt(1.f - sin_t_square);
    }

    out_wi = Vector(-wo.v.x * eta, -wo.v.y * eta, cos_t);
    out_pdf = 1.0f;
    float tau = max(0.f, 1.f - this->fresnel.reflectance(Bsdf::cos_theta(wo)));
    return this->transmittance * tau / Bsdf::abs_cos_theta(wo);
  }

  float SpecularBtdf::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }
}

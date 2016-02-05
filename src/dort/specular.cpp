#include "dort/specular.hpp"

namespace dort {
  float fresnel_conductor(float eta, float k, float cos_i) {
    float tmp_1 = square(eta) + square(k);
    float tmp_2 = tmp_1 * square(cos_i);
    float tmp_3 = 2.f * eta * cos_i;
    float r_para_sq = (tmp_2 - tmp_3 + 1.f) / (tmp_2 - tmp_3 + 1.f);
    float r_perp_sq = (tmp_1 - tmp_3 + square(cos_i)) / (tmp_1 + tmp_3 + square(cos_i));
    assert(is_finite(r_para_sq) && is_finite(r_perp_sq));
    return 0.5f * (r_para_sq + r_perp_sq);
  }

  float fresnel_dielectric(float eta_t, float eta_i, float cos_i, float cos_t) {
    float term_tt = eta_t * cos_t;
    float term_ti = eta_t * cos_i;
    float term_it = eta_i * cos_t;
    float term_ii = eta_i * cos_i;
    float r_para = (term_ti - term_it) / (term_ti + term_it);
    float r_perp = (term_ii - term_tt) / (term_ii + term_tt);
    assert(is_finite(r_para) && is_finite(r_perp));
    return 0.5f * (square(r_para) + square(r_perp));
  }

  float FresnelConductor::reflectance(float cos_i) const {
    return fresnel_conductor(this->eta, this->k, abs(cos_i));
  }

  float FresnelDielectric::reflectance(float cos_i) const {
    float eta_i, eta_t;
    if(cos_i > 0.f) {
      eta_i = this->eta_i;
      eta_t = this->eta_t;
    } else {
      eta_i = this->eta_t;
      eta_t = this->eta_i;
    }

    float sin_t_square = square(eta_i / eta_t) * (1.f - cos_i);
    if(sin_t_square > 1.f) {
      return 0.f;
    }

    float cos_t = sqrt(max(0.f, sin_t_square));
    return fresnel_dielectric(eta_t, eta_i, cos_i, cos_t);
  }

  Spectrum SpecularBrdf::f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  Spectrum SpecularBrdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, Rng&) const
  {
    float cos_theta = Bsdf::cos_theta(wo);
    if(cos_theta == 0.f) {
      return Spectrum(0.f);
    }

    out_wi = Vector(-wo.v.x, -wo.v.y, wo.v.z);
    out_pdf = 1.f;
    return this->reflectance * this->fresnel->reflectance(cos_theta) / abs(cos_theta);
  }

  float SpecularBrdf::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  Spectrum SpecularBtdf::f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  Spectrum SpecularBtdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, Rng&) const 
  {
    if(Bsdf::cos_theta(wo) == 0.f) {
      return Spectrum(0.f);
    }

    float eta;
    if(Bsdf::cos_theta(wo) > 0.f) {
      eta = this->fresnel.eta_i / this->fresnel.eta_t;
    } else {
      eta = this->fresnel.eta_t / this->fresnel.eta_i;
    }

    float sin_t_square = square(eta) * Bsdf::sin_theta_square(wo);
    if(sin_t_square > 1.f) {
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
    float tau = 1.f - this->fresnel.reflectance(Bsdf::cos_theta(wo));
    return this->transmittance * tau / Bsdf::abs_cos_theta(wo);
  }

  float SpecularBtdf::f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }
}

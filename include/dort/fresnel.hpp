#pragma once
#include "dort/dort.hpp"
#include "dort/math.hpp"

namespace dort {
  float fresnel_conductor(float eta, float k, float cos_i);
  float fresnel_dielectric(float eta_t, float eta_i, float cos_i, float cos_t);

  struct FresnelConductor {
    float eta;
    float k;
    FresnelConductor(float eta, float k):
      eta(eta), k(k) { }

    float reflectance(float cos_i) const {
      return fresnel_conductor(this->eta, this->k, abs(cos_i));
    }
    float eta_i() const { return 1.f; }
    float eta_t() const { return this->eta; }
  };

  struct FresnelDielectric {
    float eta_i_;
    float eta_t_;
    FresnelDielectric(float eta_i, float eta_t):
      eta_i_(eta_i), eta_t_(eta_t) { }

    float reflectance(float cos_i) const;
    float eta_i() const { return this->eta_i_; }
    float eta_t() const { return this->eta_t_; }
  };

  struct FresnelConstant {
    float reflect;
    float eta;

    FresnelConstant(float reflectance, float eta):
      reflect(reflectance), eta(eta) { }

    float reflectance(float) const { return this->reflect; }
    float eta_i() const { return 1.f; }
    float eta_t() const { return this->eta; }
  };
}

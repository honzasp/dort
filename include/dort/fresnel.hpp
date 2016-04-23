#pragma once
#include "dort/dort.hpp"
#include "dort/math.hpp"

namespace dort {
  float fresnel_conductor(float eta, float k, float cos_i);
  float fresnel_dielectric(float eta, float cos_i, float cos_t);

  struct Fresnel {
    float eta;
    float inv_eta;
    explicit Fresnel(float eta): eta(eta), inv_eta(1.f / eta) { }
  };

  struct FresnelConductor: public Fresnel {
    float k;
    FresnelConductor(float eta, float k):
      Fresnel(eta), k(k) { }

    float reflectance(float cos_i, float) const {
      return fresnel_conductor(this->eta, this->k, abs(cos_i));
    }
  };

  struct FresnelDielectric: public Fresnel {
    explicit FresnelDielectric(float eta): Fresnel(eta) { }

    float reflectance(float cos_i, float cos_t) const {
      return fresnel_dielectric(cos_i > 0.f ? this->eta : this->inv_eta,
          abs(cos_i), abs(cos_t));
    }
  };

  struct FresnelConstant: public Fresnel {
    float reflect;

    FresnelConstant(float eta, float reflectance):
      Fresnel(eta), reflect(reflectance) { }

    float reflectance(float, float) const { return this->reflect; }
  };
}

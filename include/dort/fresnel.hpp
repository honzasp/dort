#pragma once
#include "dort/dort.hpp"

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
}

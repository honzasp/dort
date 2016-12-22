#pragma once
#include "dort/dort.hpp"
#include "dort/math.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  float fresnel_dielectric(float cos_i, float eta_i, float eta_t);
  Spectrum fresnel_conductor(float cos_i, Spectrum eta_i, Spectrum eta_t, Spectrum k);

  struct FresnelConductor {
    Spectrum eta_outside;
    Spectrum eta_inside;
    Spectrum k_inside;
    FresnelConductor(Spectrum eta_outside, Spectrum eta_inside, Spectrum k_inside):
      eta_outside(eta_outside), eta_inside(eta_inside), k_inside(k_inside) { }

    Spectrum reflectance(float cos_i) const {
      return fresnel_conductor(cos_i,
          this->eta_outside, this->eta_inside, this->k_inside);
    }
  };

  struct FresnelDielectric {
    float eta_outside;
    float eta_inside;
    FresnelDielectric(float eta_outside, float eta_inside):
      eta_outside(eta_outside), eta_inside(eta_inside) { }

    Spectrum reflectance(float cos_i) const {
      return Spectrum(fresnel_dielectric(cos_i, this->eta_outside, this->eta_inside));
    }
  };

  struct FresnelConstant {
    float reflect;
    FresnelConstant(float reflectance): reflect(reflectance) { }

    float reflectance(float) const {
      return this->reflect; 
    }
  };
}

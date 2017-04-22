#pragma once
#include "dort/dort.hpp"

namespace dort {
  bool refract_dielectric(const Vector& w, Vector& out_w_trans,
      float ior_refl, float ior_trans);
  float fresnel_dielectric(float cos_refl, float cos_trans,
      float ior_refl, float ior_trans);
  float fresnel_dielectric_refl(float cos_refl, float& out_cos_trans,
      float ior_refl, float ior_trans);
}

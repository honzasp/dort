#include "dort/fresnel.hpp"
#include "dort/geometry.hpp"

namespace dort {
  bool refract_dielectric(const Vector& w, Vector& out_w_trans,
      float ior_refl, float ior_trans)
  {
    float eta = ior_refl / ior_trans;
    float xt = w.v.x * eta;
    float yt = w.v.y * eta;
    float sin_t_squared = square(xt) + square(yt);
    if(sin_t_squared >= 1.f) {
      return false;
    }

    float zt = sqrt(1.f - sin_t_squared);
    out_w_trans = Vector(-xt, -yt, -copysign(zt, w.v.z));
    return true;
  }

  float fresnel_dielectric(float cos_refl, float cos_trans,
      float ior_refl, float ior_trans)
  {
    assert(cos_refl >= 0.f && cos_refl <= 1.001f);
    assert(cos_trans >= 0.f && cos_trans <= 1.001f);
    float term_tr = ior_trans * cos_refl;
    float term_rt = ior_refl * cos_trans;
    float term_rr = ior_refl * cos_refl;
    float term_tt = ior_trans * cos_trans;
    float r_para = (term_tr - term_rt) / (term_tr + term_rt);
    float r_perp = (term_rr - term_tt) / (term_rr + term_tt);

    return 0.5f * (square(r_para) + square(r_perp));
  }

  float fresnel_dielectric_refl(float cos_refl, float& out_cos_trans,
      float ior_refl, float ior_trans) 
  {
    float cos_trans_square = 1.f - square(ior_refl / ior_trans) 
      * (1.f - square(cos_refl));
    if(cos_trans_square <= 0.f) {
      out_cos_trans = 0.f;
      return 1.f; 
    }
    out_cos_trans = sqrt(cos_trans_square);
    return fresnel_dielectric(cos_refl, out_cos_trans, ior_refl, ior_trans);
  }
}

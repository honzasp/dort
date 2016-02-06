#include "dort/fresnel.hpp"
#include "dort/math.hpp"

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
}

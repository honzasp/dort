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

  float fresnel_dielectric(float eta, float cos_i, float cos_t) {
    float term_tt = eta * cos_t;
    float term_ti = eta * cos_i;
    float term_it = cos_t;
    float term_ii = cos_i;
    float r_para = (term_ti - term_it) / (term_ti + term_it);
    float r_perp = (term_ii - term_tt) / (term_ii + term_tt);
    assert(is_finite(r_para) && is_finite(r_perp));
    return 0.5f * (square(r_para) + square(r_perp));
  }
}

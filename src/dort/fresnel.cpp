#include "dort/fresnel.hpp"
#include "dort/math.hpp"

namespace dort {
  float fresnel_dielectric(float cos_i, float eta_i, float eta_t) {
    if(cos_i < 0.f) {
      std::swap(eta_i, eta_t);
      cos_i = abs(cos_i);
    }

    cos_i = clamp(cos_i, 0.f, 1.f);
    float sin_i = sqrt(1.f - square(cos_i));
    float sin_t = eta_i / eta_t * sin_i;
    if(sin_t >= 1.f) {
      return 1.f;
    }
    float cos_t = sqrt(1.f - square(sin_t));

    float x_tt = eta_t * cos_t;
    float x_ti = eta_t * cos_i;
    float x_it = eta_i * cos_t;
    float x_ii = eta_i * cos_i;
    float r_para = (x_ti - x_it) / (x_ti + x_it);
    float r_perp = (x_ii - x_tt) / (x_ii + x_tt);
    return 0.5f * (square(r_para) + square(r_perp));
  }

  Spectrum fresnel_conductor(float cos_i, Spectrum eta_i, Spectrum eta_t, Spectrum k_t) {
    float cos_i2 = clamp(square(cos_i), 0.f, 1.f);
    float sin_i2 = 1.f - cos_i2;
    Spectrum eta = eta_t / eta_i;
    Spectrum eta_k = k_t / eta_i;
    Spectrum eta2 = square(eta);
    Spectrum eta_k2 = square(eta_k);

    Spectrum t0 = eta2 - eta_k2 - Spectrum(sin_i2);
    Spectrum a2_b2 = sqrt(square(t0) + 4.f * eta2 * eta_k2);
    Spectrum t1 = a2_b2 + Spectrum(cos_i2);
    Spectrum a = sqrt(0.5f * (a2_b2 + t0));
    Spectrum t2 = 2.f * a * cos_i;
    Spectrum r_perp = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cos_i2 * a2_b2 + Spectrum(square(sin_i2));
    Spectrum t4 = t2 * sin_i2;
    Spectrum r_para = r_perp * (t3 - t4) / (t3 + t4);

    return 0.5f * (r_perp + r_para);
  }
}

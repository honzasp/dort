#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  template<class G1>
  struct SmithG {
    G1 g1;
    SmithG(G1 g1): g1(std::move(g1)) { }
    float g(const Vector& wo, const Vector& wi, const Vector& m) const {
      return g1.g1(wo, m) * g1.g1(wi, m);
    }
  };

  struct BeckmannD {
    float alpha_b;

    BeckmannD(float alpha_b): alpha_b(alpha_b) { }

    float d(const Vector& m) const {
      if(Bsdf::cos_theta(m) <= 0.f) {
        return 0.f;
      }
      float tmp_1 = PI * square(this->alpha_b * Bsdf::cos_theta_square(m));
      float tmp_2 = Bsdf::sin_theta_square(m) /
        (Bsdf::cos_theta_square(m) * square(this->alpha_b));
      return 1.f / tmp_1 * exp(-tmp_2);
    }

    Vector sample_m(float u1, float u2, float& out_pdf) const {
      float tan_theta = this->alpha_b * sqrt(-log(u1));
      float phi = TWO_PI * u2;
      Vector m = normalize(Vector(cos(phi) * tan_theta, sin(phi) * tan_theta, 1.f));
      out_pdf = this->d(m) * Bsdf::cos_theta(m);
      return m;
    }

    float m_pdf(const Vector& m) const {
      return this->d(m) * Bsdf::cos_theta(m);
    }
  };

  struct BeckmannG1 {
    float alpha_b;
    BeckmannG1(float alpha_b): alpha_b(alpha_b) { }
    float g1(const Vector& v, const Vector& m) const {
      if(dot(v, m) * Bsdf::cos_theta(v) <= 0.f) {
        return 0.f;
      }
      float a = Bsdf::cos_theta(v) / (this->alpha_b * Bsdf::sin_theta(v));
      return 2.f / (1.f + erf(a) + exp(-square(a)) / (a * SQRT_PI));
    }
  };

  struct BeckmannApproxG1 {
    float alpha_b;
    BeckmannApproxG1(float alpha_b): alpha_b(alpha_b) { }
    float g1(const Vector& v, const Vector& m) const {
      if(dot(v, m) * Bsdf::cos_theta(v) <= 0.f) {
        return 0.f;
      }
      float a = Bsdf::abs_cos_theta(v) / (this->alpha_b * Bsdf::sin_theta(v));
      if(a < 1.6f) {
        return (3.535f*a + 2.181*a*a) / (1.f + 2.276f*a + 2.577f*a*a);
      } else {
        return 1.f;
      }
    }
  };
}


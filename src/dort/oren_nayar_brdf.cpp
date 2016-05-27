#include "dort/oren_nayar_brdf.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/rng.hpp"

namespace dort {
  OrenNayarBrdf::OrenNayarBrdf(const Spectrum& reflectance, float sigma):
    Bxdf(BxdfFlags(BSDF_REFLECTION | BSDF_DIFFUSE)),
    reflectance(reflectance)
  {
    this->a = 1.f - square(sigma) / (2.f * (square(sigma) + 0.33f));
    this->b = 0.45f * square(sigma) / (square(sigma) + 0.09f);
  }

  Spectrum OrenNayarBrdf::f(const Vector& wo, const Vector& wi) const {
    float cos_theta_i = Bsdf::abs_cos_theta(wi);
    float sin_theta_i = Bsdf::sin_theta(wi);
    float cos_theta_o = Bsdf::abs_cos_theta(wo);
    float sin_theta_o = Bsdf::sin_theta(wo);

    if(sin_theta_i == 0.f || sin_theta_o == 0.f) {
      return this->reflectance * INV_PI * this->a;
    } else if(cos_theta_i == 0.f && cos_theta_o == 0.f) {
      return Spectrum(0.f);
    }

    float sin_alpha;
    float tan_beta;
    if(cos_theta_i < cos_theta_o) {
      sin_alpha = sin_theta_i;
      tan_beta = sin_theta_o / cos_theta_o;
    } else {
      sin_alpha = sin_theta_o;
      tan_beta = sin_theta_i / cos_theta_i;
    }

    float cos_diff_phi = (wi.v.x * wo.v.x + wi.v.y * wo.v.y) /
      (sin_theta_i * sin_theta_o);

    return this->reflectance * INV_PI * (this->a + 
        this->b * max(0.f, cos_diff_phi) * sin_alpha * tan_beta);
  }

  Spectrum OrenNayarBrdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    out_wi = Vector(cosine_hemisphere_sample(u1, u2));
    if(wo.v.z < 0.f) {
      out_wi.v.z = -out_wi.v.z;
    }
    out_pdf = cosine_hemisphere_pdf(out_wi.v);
    return this->f(wo, out_wi);
  }

  float OrenNayarBrdf::f_pdf(const Vector& wo, const Vector& wi) const {
    if(Bsdf::same_hemisphere(wo, wi)) {
      return cosine_hemisphere_pdf(wi.v);
    } else {
      return 0.f;
    }
  }
}

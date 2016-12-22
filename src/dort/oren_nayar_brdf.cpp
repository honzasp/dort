#include "dort/oren_nayar_brdf.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/rng.hpp"

namespace dort {
  OrenNayarBrdf::OrenNayarBrdf(const Spectrum& reflectance, float sigma):
    SymmetricBxdf(BSDF_REFLECTION | BSDF_DIFFUSE),
    reflectance(reflectance)
  {
    this->a = 1.f - 0.5f * square(sigma) / (square(sigma) + 0.33f);
    this->b = 0.45f * square(sigma) / (square(sigma) + 0.09f);
  }

  Spectrum OrenNayarBrdf::eval_f(const Vector& wi_light, const Vector& wo_camera) const {
    float cos_theta_i = Bsdf::abs_cos_theta(wi_light);
    float sin_theta_i = Bsdf::sin_theta(wi_light);
    float cos_theta_o = Bsdf::abs_cos_theta(wo_camera);
    float sin_theta_o = Bsdf::sin_theta(wo_camera);

    if(cos_theta_i < 1e-3 || cos_theta_o < 1e-3) {
      return Spectrum();
    }

    float cos_phi_diff = abs(sin_theta_i * sin_theta_o) > 1e-3f
      ? (wi_light.v.x * wo_camera.v.x + wi_light.v.y * wo_camera.v.y) 
        / (sin_theta_i * sin_theta_o) : 0.f;
    if(cos_phi_diff <= 0.f) {
      return this->reflectance * (INV_PI * this->a);
    }

    float sin_alpha, tan_beta;
    if(cos_theta_i >= cos_theta_o) {
      sin_alpha = sin_theta_o;
      tan_beta = sin_theta_i / cos_theta_i;
    } else {
      sin_alpha = sin_theta_i;
      tan_beta = sin_theta_o / cos_theta_o;
    }
    return this->reflectance * (INV_PI * (this->a +
        this->b * cos_phi_diff * sin_alpha * tan_beta));
  }

  Spectrum OrenNayarBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
    out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
    if(w_fix.v.z < 0.f) {
      out_w_gen.v.z = -out_w_gen.v.z;
    }
    return this->eval_f(w_fix, out_w_gen);
  }

  float OrenNayarBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector& w_fix) const {
    if(!Bsdf::same_hemisphere(w_gen, w_fix)) {
      return 0.f;
    }
    return cosine_hemisphere_pdf(w_gen.v.z);
  }
}

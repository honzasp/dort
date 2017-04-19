#include "dort/lambert_material.hpp"
#include "dort/oren_nayar_material.hpp"

namespace dort {
  Spectrum OrenNayarBrdf::eval_f(const Vector& wi_light, const Vector& wo_camera) const {
    float sigma_square = square(this->sigma);
    float a = 1.f - 0.5f * sigma_square / (sigma_square + 0.33f);
    float b = 0.45f * sigma_square / (sigma_square + 0.09f);

    float cos_theta_i = Bsdf::abs_cos_theta(wi_light);
    float cos_theta_o = Bsdf::abs_cos_theta(wo_camera);
    float sin_theta_i = Bsdf::sin_theta(wi_light);
    float sin_theta_o = Bsdf::sin_theta(wo_camera);
    float sin_phi_i = Bsdf::sin_phi(wi_light);
    float sin_phi_o = Bsdf::sin_phi(wo_camera);
    float cos_phi_i = Bsdf::cos_phi(wi_light);
    float cos_phi_o = Bsdf::cos_phi(wo_camera);

    float cos_diff_phi = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;
    float sin_alpha, tan_beta;
    if(cos_theta_i >= cos_theta_o) {
      sin_alpha = sin_theta_o;
      tan_beta = sin_theta_i / cos_theta_i;
    } else {
      sin_alpha = sin_theta_i;
      tan_beta = sin_theta_o / cos_theta_o;
    }

    return this->albedo * INV_PI * 
      (a + b * max(0.f, cos_diff_phi) * sin_alpha * tan_beta);
  }

  Spectrum OrenNayarBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
    out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    return this->eval_f(w_fix, out_w_gen);
  }

  float OrenNayarBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector&) const {
    return cosine_hemisphere_pdf(w_gen.v.z);
  }

  void OrenNayarMaterial::add_bxdfs(const DiffGeom& geom,
      Spectrum scale, Bsdf& bsdf) const
  {
    Spectrum albedo = this->albedo->evaluate(geom);
    if(albedo.is_black()) { return; }

    float sigma = this->sigma->evaluate(geom);
    if(sigma != 0.f) {
      bsdf.add(std::make_unique<OrenNayarBrdf>(albedo * scale, sigma));
    } else {
      bsdf.add(std::make_unique<LambertBrdf>(albedo * scale));
    }
  }
}

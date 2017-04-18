#include "dort/dielectric_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum DielectricBxdf::eval_f(const Vector&, const Vector&) const {
    return Spectrum(0.f); 
  }

  Spectrum DielectricBxdf::sample_light_f(const Vector& wo_camera,
      Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_f(wo_camera, out_wi_light, out_dir_pdf, uv.x, false);
  }

  Spectrum DielectricBxdf::sample_camera_f(const Vector& wi_light,
      Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_f(wi_light, out_wo_camera, out_dir_pdf, uv.x, false);
  }

  float DielectricBxdf::light_f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  float DielectricBxdf::camera_f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  Spectrum DielectricBxdf::sample_f(const Vector& w_fix, Vector& out_w_gen,
      float& out_dir_pdf, float u, bool fix_is_light) const 
  {
    float ior_refl, ior_trans;
    this->get_iors(w_fix, ior_refl, ior_trans);

    float fr = this->fresnel(Bsdf::abs_cos_theta(w_fix), ior_refl, ior_trans);
    if(fr >= u) {
      out_dir_pdf = fr;
      out_w_gen = this->reflect_w(w_fix);
      return this->reflect_tint * (fr / Bsdf::abs_cos_theta(w_fix));
    }

    out_w_gen = this->transmit_w(w_fix, ior_refl, ior_trans);
    out_dir_pdf = 1.f - fr;
    if(fix_is_light) {
      float factor = square(ior_trans / ior_refl);
      return this->transmit_tint * factor * (1.f - fr) / Bsdf::abs_cos_theta(w_fix);
    } else {
      return this->transmit_tint * (1.f - fr) / Bsdf::abs_cos_theta(out_w_gen);
    }
  }

  void DielectricBxdf::get_iors(const Vector& w,
      float& out_ior_refl, float& out_ior_trans) const 
  {
    if(w.v.z >= 0.f) {
      out_ior_refl = this->ior_outside;
      out_ior_trans = this->ior_inside;
    } else {
      out_ior_refl = this->ior_inside;
      out_ior_trans = this->ior_outside;
    }
  }

  Vector DielectricBxdf::transmit_w(const Vector& w,
      float ior_refl, float ior_trans) const 
  {
    float eta = ior_refl / ior_trans;
    float xt = w.v.x * eta;
    float yt = w.v.y * eta;
    float sin_t_squared = min(0.999f, square(xt) + square(yt));

    float zt = sqrt(1.f - sin_t_squared);
    return Vector(xt, yt, -copysign(zt, w.v.z));
  }

  Vector DielectricBxdf::reflect_w(const Vector& w) const {
    return Vector(-w.v.x, -w.v.y, w.v.z);
  }

  float DielectricBxdf::fresnel(float cos_refl, float ior_refl, float ior_trans) const {
    float sin_trans_square = square(ior_refl / ior_trans) * (1.f - square(cos_refl));
    if(sin_trans_square >= 1.f) { return 1.f; }
    float cos_trans = sqrt(1.f - sin_trans_square);

    float term_tr = ior_trans * cos_refl;
    float term_rt = ior_refl * cos_trans;
    float term_rr = ior_refl * cos_refl;
    float term_tt = ior_trans * cos_trans;
    float r_para = (term_tr - term_rt) / (term_tr + term_rt);
    float r_perp = (term_rr - term_tt) / (term_rr + term_tt);

    return 0.5f * (square(r_para) + square(r_perp));
  }

  void DielectricMaterial::add_bxdfs(const DiffGeom& geom,
      Spectrum scale, Bsdf& bsdf) const
  {
    Spectrum reflect = this->reflect_tint->evaluate(geom);
    Spectrum transmit = this->transmit_tint->evaluate(geom);
    if(reflect.is_black() && transmit.is_black()) { return; }

    float ior_inside = this->ior_inside->evaluate(geom);
    float ior_outside = this->ior_outside->evaluate(geom);
    if(ior_inside <= 0.f || ior_outside <= 0.f) { return; }

    bsdf.add(std::make_unique<DielectricBxdf>(
          reflect * scale, transmit * scale, ior_inside, ior_outside));
  }
}

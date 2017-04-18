#include "dort/dielectric_material.hpp"
#include "dort/fresnel.hpp"
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

    float cos_refl = Bsdf::abs_cos_theta(w_fix);
    Vector w_reflect(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    Vector w_transmit;
    if(!refract_dielectric(w_fix, w_transmit, ior_refl, ior_trans)) {
      out_dir_pdf = 1.f;
      out_w_gen = w_reflect;
      return this->reflect_tint / cos_refl;
    }

    float cos_trans = Bsdf::abs_cos_theta(w_transmit);
    float fr = fresnel_dielectric(cos_refl, cos_trans, ior_refl, ior_trans);
    float r = this->is_thin
      ? 1.f - square(1 - fr) / (1.f - square(fr)) : fr;
    float t = 1.f - r;

    if(u <= r) {
      out_dir_pdf = r;
      out_w_gen = w_reflect;
      return this->reflect_tint * (r / cos_refl);
    } else if(this->is_thin) {
      out_dir_pdf = t;
      out_w_gen = -w_fix;
      return this->transmit_tint * (t / cos_refl);
    } else {
      out_w_gen = w_transmit;
      out_dir_pdf = t;
      return this->transmit_tint * (fix_is_light
          ? square(ior_trans / ior_refl) * t / cos_refl
          : t / cos_trans);
    }
  }

  void DielectricBxdf::get_iors(const Vector& w,
      float& out_ior_refl, float& out_ior_trans) const 
  {
    if(this->is_thin || w.v.z >= 0.f) {
      out_ior_refl = this->ior_outside;
      out_ior_trans = this->ior_inside;
    } else {
      out_ior_refl = this->ior_inside;
      out_ior_trans = this->ior_outside;
    }
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

    bsdf.add(std::make_unique<DielectricBxdf>(reflect * scale, transmit * scale,
        ior_inside, ior_outside, this->is_thin));
  }
}

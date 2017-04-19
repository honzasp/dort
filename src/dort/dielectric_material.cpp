#include "dort/dielectric_material.hpp"
#include "dort/fresnel.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum DielectricBxdf::eval_f(const Vector&, const Vector&, BxdfFlags) const {
    return Spectrum(0.f); 
  }

  Spectrum DielectricBxdf::sample_light_f(const Vector& wo_camera, BxdfFlags request,
      Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags, Vec2 uv) const
  {
    return this->sample_f(wo_camera, request,
        out_wi_light, out_dir_pdf, out_flags, uv.x, false);
  }

  Spectrum DielectricBxdf::sample_camera_f(const Vector& wi_light, BxdfFlags request,
      Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags, Vec2 uv) const
  {
    return this->sample_f(wi_light, request,
        out_wo_camera, out_dir_pdf, out_flags, uv.x, false);
  }

  float DielectricBxdf::light_f_pdf(const Vector&, const Vector&, BxdfFlags) const {
    return 0.f;
  }

  float DielectricBxdf::camera_f_pdf(const Vector&, const Vector&, BxdfFlags) const {
    return 0.f;
  }

  Spectrum DielectricBxdf::sample_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
      float u, bool fix_is_light) const 
  {
    assert(request & BSDF_DELTA);
    bool sample_reflect = request & BSDF_REFLECTION;
    bool sample_transmit = request & BSDF_TRANSMISSION;
    assert(sample_reflect || sample_transmit);

    float ior_refl, ior_trans;
    this->get_iors(w_fix, ior_refl, ior_trans);

    float cos_refl = Bsdf::abs_cos_theta(w_fix);
    Vector w_reflect(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    Vector w_transmit;
    if(!refract_dielectric(w_fix, w_transmit, ior_refl, ior_trans)) {
      if(!sample_reflect) {
        out_flags = BSDF_NONE;
        out_dir_pdf = 0.f;
        return Spectrum(0.f);
      }
      out_flags = BSDF_DELTA | BSDF_REFLECTION;
      out_dir_pdf = 1.f;
      out_w_gen = w_reflect;
      return this->reflect_tint / cos_refl;
    }

    float cos_trans = Bsdf::abs_cos_theta(w_transmit);
    float fr = fresnel_dielectric(cos_refl, cos_trans, ior_refl, ior_trans);
    float r = this->is_thin
      ? 1.f - square(1 - fr) / (1.f - square(fr)) : fr;
    float t = 1.f - r;

    bool reflect = u <= r && sample_reflect;
    bool transmit = u > r && sample_transmit;
    float reflect_pdf = sample_transmit ? r : 1.f;
    float transmit_pdf = sample_reflect ? t : 1.f;

    if(reflect) {
      out_flags = BSDF_DELTA | BSDF_REFLECTION;
      out_dir_pdf = reflect_pdf;
      out_w_gen = w_reflect;
      return this->reflect_tint * (r / cos_refl);
    } else if(transmit && this->is_thin) {
      out_flags = BSDF_DELTA | BSDF_TRANSMISSION;
      out_dir_pdf = transmit_pdf;
      out_w_gen = -w_fix;
      return this->transmit_tint * (t / cos_refl);
    } else if(transmit) {
      out_flags = BSDF_DELTA | BSDF_TRANSMISSION;
      out_dir_pdf = transmit_pdf;
      out_w_gen = w_transmit;
      return this->transmit_tint * (fix_is_light
          ? square(ior_trans / ior_refl) * t / cos_refl
          : t / cos_trans);
    } else {
      out_flags = BSDF_NONE;
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
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

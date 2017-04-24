#include "dort/dielectric_material.hpp"
#include "dort/fresnel.hpp"
#include "dort/rough_dielectric_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum RoughDielectricBxdf::eval_f(const Vector& wi_light,
      const Vector& wo_camera, BxdfFlags request) const
  {
    assert(request & BSDF_GLOSSY);
    bool reflect = request & BSDF_REFLECTION;
    float ior_refl, ior_trans;
    this->get_iors(wi_light, ior_refl, ior_trans);

    Vector h;
    if(reflect) {
      h = normalize(wi_light + wo_camera);
    } else {
      h = -normalize(ior_refl * wi_light + ior_trans * wo_camera);
    }

    float wi_dot_n = Bsdf::abs_cos_theta(wi_light);
    float wo_dot_n = Bsdf::abs_cos_theta(wo_camera);
    float wi_dot_h = abs_dot(wi_light, h);
    float wo_dot_h = abs_dot(wo_camera, h);
    float fr = fresnel_dielectric(wi_dot_h, wo_dot_h, ior_refl, ior_trans);
    float g = this->distrib.g(wi_light, wo_camera, h);
    float d = this->distrib.d(h);

    if(reflect) {
      return this->reflect_tint * (fr * g * d * 0.25f / (wi_dot_n * wo_dot_n));
    } else {
      return this->transmit_tint * (
        wi_dot_h * wo_dot_h * square(ior_trans) * (1.f - fr) * g * d
        / (wi_dot_n * wo_dot_n * square(ior_refl * wi_dot_h - ior_trans * wo_dot_h)));
    }
  }

  Spectrum RoughDielectricBxdf::sample_light_f(const Vector& wo_camera,
      BxdfFlags request, Vector& out_wi_light, float& out_dir_pdf,
      BxdfFlags& out_flags, Vec3 uvc) const
  {
    return this->sample_f(wo_camera, request, out_wi_light, out_dir_pdf,
        out_flags, uvc);
  }

  Spectrum RoughDielectricBxdf::sample_camera_f(const Vector& wi_light,
      BxdfFlags request, Vector& out_wo_camera, float& out_dir_pdf,
      BxdfFlags& out_flags, Vec3 uvc) const
  {
    return this->sample_f(wi_light, request, out_wo_camera, out_dir_pdf,
        out_flags, uvc);
  }

  float RoughDielectricBxdf::light_f_pdf(const Vector& wi_light_gen,
      const Vector& wo_camera_fix, BxdfFlags request) const
  {
    return this->f_pdf(wi_light_gen, wo_camera_fix, request);
  }

  float RoughDielectricBxdf::camera_f_pdf(const Vector& wo_camera_gen,
      const Vector& wi_light_fix, BxdfFlags request) const
  {
    return this->f_pdf(wo_camera_gen, wi_light_fix, request);
  }

  Spectrum RoughDielectricBxdf::sample_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags, Vec3 uvc) const 
  {
    assert(request & BSDF_GLOSSY);
    bool sample_refl = request & BSDF_REFLECTION;
    bool sample_trans = request & BSDF_TRANSMISSION;
    float ior_refl, ior_trans;
    this->get_iors(w_fix, ior_refl, ior_trans);

    float m_dir_pdf;
    Vector m = this->distrib.sample_m(Vec2(uvc.x, uvc.y), m_dir_pdf);
    m.v.z = copysign(m.v.z, w_fix.v.z);

    float wfix_dot_m = dot(w_fix, m);
    float wgen_dot_m_trans;
    float fr = fresnel_dielectric_refl(abs(wfix_dot_m), wgen_dot_m_trans,
        ior_refl, ior_trans);
    wgen_dot_m_trans = -copysign(wgen_dot_m_trans, wfix_dot_m);

    bool reflect;
    float pick_pdf;
    if(sample_refl && sample_trans) {
      if((reflect = uvc.z <= fr)) {
        pick_pdf = fr;
      } else {
        pick_pdf = 1.f - fr;
      }
    } else if(sample_refl) {
      reflect = true;
      pick_pdf = 1.f;
    } else if(sample_trans && fr < 1.f) {
      reflect = false;
      pick_pdf = 1.f;
    } else {
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
    }

    Vector w_gen;
    float base_pdf;
    Spectrum base_f;
    BxdfFlags base_flags;
    if(reflect) {
      w_gen = 2.f * dot(w_fix, m) * m - w_fix;
      base_pdf = m_dir_pdf * 0.25f / abs(wfix_dot_m);
      base_f = this->reflect_tint * (0.25f * fr);
      base_flags = BSDF_REFLECTION;
    } else {
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
      /*
      float eta = ior_refl / ior_trans;
      float jacobian = square(ior_trans) * abs(wgen_dot_m_trans)
        / square(ior_refl * wfix_dot_m + ior_trans * wgen_dot_m_trans);
      w_gen = (eta * wfix_dot_m 
        - sign(w_fix.v.z) * sqrt(1.f + eta * (square(wfix_dot_m) - 1.f))) * m
        + eta * w_fix;
      base_pdf = m_dir_pdf * jacobian;
      base_f = this->transmit_tint * (jacobian * (1.f - fr) * abs(wfix_dot_m));
      base_flags = BSDF_TRANSMISSION;
      */
    }
    assert(is_finite(base_f)); assert(is_nonnegative(base_f));
    assert(is_finite(base_pdf)); assert(base_pdf >= 0.f);

    if(base_pdf < 1e-5f) {
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
    }

    float g = this->distrib.g(w_fix, w_gen, m);
    float d = this->distrib.d(m);
    float wfix_dot_n = Bsdf::cos_theta(w_fix);
    float wgen_dot_n = Bsdf::cos_theta(w_gen);
    assert(is_finite(g)); assert(is_finite(d));

    out_w_gen = w_gen;
    out_dir_pdf = pick_pdf * base_pdf;
    out_flags = BSDF_GLOSSY | base_flags;
    return base_f * (g * d / abs(wfix_dot_n * wgen_dot_n));
  }

  float RoughDielectricBxdf::f_pdf(const Vector& w_gen,
      const Vector& w_fix, BxdfFlags request) const
  {
    // TODO
    (void)w_gen; (void)w_fix; (void)request;
    return 0.f;
  }

  void RoughDielectricBxdf::get_iors(const Vector& w,
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
  
  void RoughDielectricMaterial::add_bxdfs(const DiffGeom& geom,
      Spectrum scale, Bsdf& bsdf) const
  {
    auto reflect = this->reflect_tint->evaluate(geom);
    auto transmit = this->transmit_tint->evaluate(geom);
    if(reflect.is_black() && transmit.is_black()) { return; }

    float roughness = this->roughness->evaluate(geom);
    if(roughness < 0.001) {
      bsdf.add(std::make_unique<DielectricBxdf>(reflect * scale, transmit * scale,
          this->ior_inside, this->ior_outside, false));
    } else {
      bsdf.add(std::make_unique<RoughDielectricBxdf>(reflect * scale, transmit * scale,
          this->ior_inside, this->ior_outside,
          MicrofacetDistrib(this->distribution, roughness)));
    }
  }
}


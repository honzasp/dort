#include "dort/fresnel.hpp"
#include "dort/specular.hpp"

namespace dort {
  template<class F>
  Spectrum MirrorBrdf<F>::eval_f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  template<class F>
  Spectrum MirrorBrdf<F>::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2) const
  {
    out_w_gen = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    out_dir_pdf = 1.f;
    float cos_theta = Bsdf::cos_theta(w_fix);
    return this->reflectance * (this->fresnel.reflectance(cos_theta) / cos_theta);
  }

  template<class F>
  float MirrorBrdf<F>::symmetric_f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  template class MirrorBrdf<FresnelConductor>;
  template class MirrorBrdf<FresnelDielectric>;
  template class MirrorBrdf<FresnelConstant>;

  Spectrum FresnelBxdf::eval_f(const Vector&, const Vector&) const {
    return Spectrum(0.f);
  }

  Spectrum FresnelBxdf::sample_light_f(const Vector& wo_camera,
      Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_f<true>(wo_camera, out_wi_light, out_dir_pdf, uv);
  }

  Spectrum FresnelBxdf::sample_camera_f(const Vector& wi_light,
      Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_f<false>(wi_light, out_wo_camera, out_dir_pdf, uv);
  }

  template<bool WI_IS_CAMERA>
  Spectrum FresnelBxdf::sample_f(const Vector& wi,
      Vector& out_wt, float& out_dir_pdf, Vec2 uv) const
  {
    float cos_i = Bsdf::cos_theta(wi);
    float eta_i, eta_t;
    if(cos_i > 0.f) {
      eta_i = this->eta_outside;
      eta_t = this->eta_inside;
    } else {
      eta_i = this->eta_inside;
      eta_t = this->eta_outside;
    }

    Vector transmit_wt;
    if(!FresnelBxdf::refract(wi, eta_i, eta_t, transmit_wt)) {
      out_wt = Vector(-wi.v.x, -wi.v.y, wi.v.z);
      out_dir_pdf = 1.f;
      return this->reflectance / abs(cos_i);
    }

    Spectrum fr = this->fresnel.reflectance(cos_i);
    Spectrum reflect = fr * this->reflectance;
    Spectrum transmit = (Spectrum(1.f) - fr) * this->transmittance;

    float reflect_weight = reflect.average();
    float transmit_weight = transmit.average();
    float threshold = reflect_weight / (reflect_weight + transmit_weight);
    if(uv.x < threshold) {
      out_wt = Vector(-wi.v.x, -wi.v.y, wi.v.z);
      out_dir_pdf = threshold;
      return reflect / abs(cos_i);
    } else {
      out_wt = transmit_wt;
      out_dir_pdf = 1.f - threshold;
      if(WI_IS_CAMERA) {
        transmit *= square(eta_t / eta_i);
      }
      return transmit / Bsdf::abs_cos_theta(transmit_wt);
    }
  }

  float FresnelBxdf::light_f_pdf(const Vector&, const Vector&) const {
    return 0.f; 
  }
  float FresnelBxdf::camera_f_pdf(const Vector&, const Vector&) const {
    return 0.f;
  }

  bool FresnelBxdf::refract(Vector wi, float eta_i, float eta_t, Vector& out_wt) {
    float eta = eta_i / eta_t;
    float cos_t_square = 1.f + square(eta) * (square(wi.v.z) - 1.f);
    if(cos_t_square <= 0.f) {
      return false;
    }
    out_wt = Vector(-eta * wi.v.x, -eta * wi.v.y,
        -copysign(sqrt(cos_t_square), wi.v.z));
    return true;
  }
}

#pragma once
#include "dort/bsdf.hpp"
#include "dort/fresnel.hpp"

namespace dort {
  template<class F>
  class MirrorBrdf final: public SymmetricBxdf {
    Spectrum reflectance;
    F fresnel;
  public:
    MirrorBrdf(const Spectrum& reflectance, F fresnel):
      SymmetricBxdf(BSDF_REFLECTION | BSDF_DELTA),
      reflectance(reflectance), fresnel(std::move(fresnel)) { }

    virtual Spectrum eval_f(const Vector& wi_light,
        const Vector& wo_camera) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final;
  };

  class FresnelBxdf final: public Bxdf {
    Spectrum reflectance;
    Spectrum transmittance;
    FresnelDielectric fresnel;
    float eta_outside;
    float eta_inside;
  public:
    FresnelBxdf(const Spectrum& reflectance, const Spectrum& transmittance,
        FresnelDielectric fresnel, float eta_outside, float eta_inside):
      Bxdf(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_DELTA),
      reflectance(reflectance),
      transmittance(transmittance),
      fresnel(std::move(fresnel)),
      eta_outside(eta_outside), eta_inside(eta_inside)
    { }

    virtual Spectrum eval_f(const Vector& wi_light,
        const Vector& wo_camera) const override final;
    virtual Spectrum sample_light_f(const Vector& wo_camera,
        Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const override final;
    virtual Spectrum sample_camera_f(const Vector& wi_light,
        Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix) const override final;
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix) const override final;

  private:
    template<bool WI_IS_CAMERA>
    Spectrum sample_f(const Vector& wi, Vector& out_wt,
        float& out_dir_pdf, Vec2 uv) const;

    static bool refract(Vector wi, float eta_i, float eta_t, Vector& out_wt);
  };

}

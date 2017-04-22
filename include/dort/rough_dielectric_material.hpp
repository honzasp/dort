#pragma once
#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/microfacet.hpp"

namespace dort {
  class RoughDielectricBxdf final: public Bxdf {
    Spectrum reflect_tint;
    Spectrum transmit_tint;
    float ior_inside;
    float ior_outside;
    MicrofacetDistrib distrib;
  public:
    RoughDielectricBxdf(Spectrum reflect_tint, Spectrum transmit_tint,
        float ior_inside, float ior_outside, MicrofacetDistrib distrib):
      Bxdf(BSDF_GLOSSY | BSDF_REFLECTION | BSDF_TRANSMISSION),
      reflect_tint(reflect_tint), transmit_tint(transmit_tint),
      ior_inside(ior_inside), ior_outside(ior_outside), distrib(distrib)
    { }

    virtual Spectrum eval_f(const Vector& wi_light, const Vector& wo_camera,
        BxdfFlags request) const override final;
    virtual Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags request,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const override final;
    virtual Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags request,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const override final;
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix, BxdfFlags request) const override final;
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix, BxdfFlags request) const override final;

  private:
    Spectrum sample_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags, Vec3 uvc) const;
    float f_pdf(const Vector& w_gen, const Vector& w_fix, BxdfFlags request) const;
    void get_iors(const Vector& w, float& out_ior_refl, float& out_ior_trans) const;
  };

  class RoughDielectricMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflect_tint;
    std::shared_ptr<TextureGeom<Spectrum>> transmit_tint;
    std::shared_ptr<TextureGeom<float>> roughness;
    float ior_inside, ior_outside;
    MicrofacetType distribution;
  public:
    RoughDielectricMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> reflect_tint,
        std::shared_ptr<TextureGeom<Spectrum>> transmit_tint,
        std::shared_ptr<TextureGeom<float>> roughness,
        float ior_inside, float ior_outside,
        MicrofacetType distribution):
      reflect_tint(reflect_tint), transmit_tint(transmit_tint),
      roughness(roughness), ior_inside(ior_inside),
      ior_outside(ior_outside), distribution(distribution)
    { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

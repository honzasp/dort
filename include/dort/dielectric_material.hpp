#pragma once
#include "dort/bsdf.hpp"
#include "dort/material.hpp"

namespace dort {
  class DielectricBxdf final: public Bxdf {
    Spectrum reflect_tint;
    Spectrum transmit_tint;
    float ior_inside;
    float ior_outside;
    bool is_thin;
  public:
    DielectricBxdf(const Spectrum& reflect_tint, const Spectrum& transmit_tint,
        float ior_inside, float ior_outside, bool is_thin):
      Bxdf(BSDF_DELTA | BSDF_REFLECTION | BSDF_TRANSMISSION),
      reflect_tint(reflect_tint), transmit_tint(transmit_tint),
      ior_inside(ior_inside), ior_outside(ior_outside), is_thin(is_thin)
    { }

    virtual Spectrum eval_f(const Vector&, const Vector&,
        BxdfFlags) const override final;
    virtual Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags request,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec2 uv) const override final;
    virtual Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags request,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec2 uv) const override final;
    virtual float light_f_pdf(const Vector&, const Vector&,
        BxdfFlags request) const override final;
    virtual float camera_f_pdf(const Vector&, const Vector&,
        BxdfFlags request) const override final;
  private:
    Spectrum sample_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        float u, bool fix_is_light) const;
    void get_iors(const Vector& w, float& out_ior_refl, float& out_ior_trans) const;
  };

  class DielectricMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> reflect_tint;
    std::shared_ptr<TextureGeom<Spectrum>> transmit_tint;
    std::shared_ptr<TextureGeom<float>> ior_inside;
    std::shared_ptr<TextureGeom<float>> ior_outside;
    bool is_thin;
  public:
    DielectricMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> reflect_tint,
        std::shared_ptr<TextureGeom<Spectrum>> transmit_tint,
        std::shared_ptr<TextureGeom<float>> ior_inside,
        std::shared_ptr<TextureGeom<float>> ior_outside,
        bool is_thin):
      reflect_tint(reflect_tint), transmit_tint(transmit_tint),
      ior_inside(ior_inside), ior_outside(ior_outside), is_thin(is_thin)
    { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

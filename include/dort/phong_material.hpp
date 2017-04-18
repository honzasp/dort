#pragma once
#include "dort/material.hpp"
#include "dort/texture.hpp"

namespace dort {
  class PhongBrdf final: public SymmetricBxdf {
    Spectrum diffuse_albedo;
    Spectrum glossy_albedo;
    float exponent;
    float glossy_pdf;
  public:
    PhongBrdf(const Spectrum& diffuse_albedo,
        const Spectrum& glossy_albedo, float exponent);

    virtual Spectrum eval_f(const Vector& wi_light,
        const Vector& wo_camera) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final;
  };

  class PhongMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> diffuse_albedo;
    std::shared_ptr<TextureGeom<Spectrum>> glossy_albedo;
    std::shared_ptr<TextureGeom<float>> exponent;
  public:
    PhongMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> diffuse_albedo,
        std::shared_ptr<TextureGeom<Spectrum>> glossy_albedo,
        std::shared_ptr<TextureGeom<float>> exponent):
      diffuse_albedo(diffuse_albedo),
      glossy_albedo(glossy_albedo),
      exponent(exponent)
    { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

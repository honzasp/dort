#pragma once
#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/texture.hpp"

namespace dort {
  class LambertBrdf final: public SymmetricBxdf {
    Spectrum albedo;
  public:
    LambertBrdf(const Spectrum& albedo):
      SymmetricBxdf(BSDF_REFLECTION | BSDF_DIFFUSE),
      albedo(albedo)
    { }

    virtual Spectrum eval_f(const Vector&, const Vector&) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final;
  };

  class LambertMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> albedo;
  public:
    LambertMaterial(std::shared_ptr<TextureGeom<Spectrum>> albedo):
      albedo(albedo) { }
    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

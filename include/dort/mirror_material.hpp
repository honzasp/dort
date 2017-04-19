#pragma once
#include "dort/bsdf.hpp"
#include "dort/material.hpp"

namespace dort {
  class MirrorBrdf final: public SymmetricBxdf {
    Spectrum albedo;
  public:
    MirrorBrdf(const Spectrum& albedo):
      SymmetricBxdf(BSDF_DELTA | BSDF_REFLECTION),
      albedo(albedo) { }

    virtual Spectrum eval_f(const Vector&, const Vector&,
        BxdfFlags request) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec2) const override final;
    virtual float symmetric_f_pdf(const Vector&, const Vector&,
        BxdfFlags request) const override final;
  };

  class MirrorMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> albedo;
  public:
    MirrorMaterial(std::shared_ptr<TextureGeom<Spectrum>> albedo):
      albedo(albedo) { }
    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

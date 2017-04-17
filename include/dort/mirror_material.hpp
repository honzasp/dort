#pragma once
#include "dort/material.hpp"

namespace dort {
  class MirrorBrdf final: public SymmetricBxdf {
    Spectrum albedo;
  public:
    MirrorBrdf(const Spectrum& albedo):
      SymmetricBxdf(BSDF_DELTA | BSDF_REFLECTION),
      albedo(albedo) { }

    virtual Spectrum eval_f(const Vector&, const Vector&) const override final {
      return Spectrum(0.f);
    }

    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2) const override final
    {
      out_w_gen = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
      out_dir_pdf = 1.f;
      return this->albedo;
    }

    virtual float symmetric_f_pdf(const Vector&, const Vector&) const override final {
      return 0.f;
    }
  };

  class MirrorMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> albedo;
  public:
    MirrorMaterial(std::shared_ptr<TextureGeom<Spectrum>> albedo):
      albedo(albedo) { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final
    {
      Spectrum albedo = this->albedo->evaluate(geom);
      if(!albedo.is_black()) {
        bsdf.add(std::make_unique<MirrorBrdf>(albedo * scale));
      }
    }
  };
}

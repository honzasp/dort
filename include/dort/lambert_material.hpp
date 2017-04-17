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

    virtual Spectrum eval_f(const Vector&, const Vector&) const override final {
      return this->albedo * INV_PI;
    }

    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final
    {
      out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
      out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
      out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
      return this->albedo * INV_PI;
    }

    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final
    {
      if(!Bsdf::same_hemisphere(w_gen, w_fix)) { return 0.f; }
      return cosine_hemisphere_pdf(w_gen.v.z);
    }
  };

  class LambertMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> albedo;
  public:
    LambertMaterial(std::shared_ptr<TextureGeom<Spectrum>> albedo):
      albedo(albedo) { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final
    {
      Spectrum albedo = this->albedo->evaluate(geom);
      if(!albedo.is_black()) {
        bsdf.add(std::make_unique<LambertBrdf>(albedo * scale));
      }
    }
  };
}

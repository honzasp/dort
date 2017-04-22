#pragma once
#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/texture.hpp"

namespace dort {
  class OrenNayarBrdf final: public SymmetricBxdf {
    Spectrum albedo;
    float sigma;
  public:
    OrenNayarBrdf(const Spectrum& albedo, float sigma):
      SymmetricBxdf(BSDF_REFLECTION | BSDF_DIFFUSE),
      albedo(albedo), sigma(sigma)
    { }

    virtual Spectrum eval_f(const Vector&, const Vector&,
        BxdfFlags request) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const override final;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix, BxdfFlags request) const override final;
  };

  class OrenNayarMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> albedo;
    std::shared_ptr<TextureGeom<float>> sigma;
  public:
    OrenNayarMaterial(std::shared_ptr<TextureGeom<Spectrum>> albedo,
        std::shared_ptr<TextureGeom<float>> sigma):
      albedo(albedo), sigma(sigma) { }
    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final;
  };
}

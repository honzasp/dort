#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  class OrenNayarBrdf final: public SymmetricBxdf {
    Spectrum reflectance;
    float a;
    float b;
  public:
    OrenNayarBrdf(const Spectrum& reflectance, float sigma);

    virtual Spectrum eval_f(const Vector& wi_light,
        const Vector& wo_camera) const override final;
    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final;
  };
}

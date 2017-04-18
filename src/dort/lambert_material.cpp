#include "dort/lambert_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum LambertBrdf::eval_f(const Vector&, const Vector&) const {
    return this->albedo * INV_PI;
  }

  Spectrum LambertBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
    out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    return this->albedo * INV_PI;
  }

  float LambertBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector&) const {
    return cosine_hemisphere_pdf(w_gen.v.z);
  }

  void LambertMaterial::add_bxdfs(const DiffGeom& geom,Spectrum scale,Bsdf& bsdf) const {
    Spectrum albedo = this->albedo->evaluate(geom);
    if(!albedo.is_black()) {
      bsdf.add(std::make_unique<LambertBrdf>(albedo * scale));
    }
  }
}

#include "dort/mirror_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum MirrorBrdf::eval_f(const Vector&, const Vector&, BxdfFlags) const {
    return Spectrum(0.f);
  }

  Spectrum MirrorBrdf::sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags, Vec3) const
  {
    assert((request & BSDF_DELTA) && (request & BSDF_REFLECTION));
    out_w_gen = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    out_dir_pdf = 1.f;
    out_flags = BSDF_DELTA | BSDF_REFLECTION;
    if(w_fix.v.z == 0.f) { return Spectrum(0.f); }
    return this->albedo / abs(w_fix.v.z);
  }

  float MirrorBrdf::symmetric_f_pdf(const Vector&, const Vector&, BxdfFlags) const {
    return 0.f;
  }

  void MirrorMaterial::add_bxdfs(const DiffGeom& geom, Spectrum scale, Bsdf& bsdf) const {
    Spectrum albedo = this->albedo->evaluate(geom);
    if(!albedo.is_black()) {
      bsdf.add(std::make_unique<MirrorBrdf>(albedo * scale));
    }
  }
}

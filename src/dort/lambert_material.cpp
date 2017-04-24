#include "dort/lambert_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  Spectrum LambertBrdf::eval_f(const Vector&, const Vector&, BxdfFlags request) const {
    assert(request & BSDF_REFLECTION); assert(request & BSDF_DIFFUSE); (void)request;
    return this->albedo * INV_PI;
  }

  Spectrum LambertBrdf::sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags, Vec3 uvc) const
  {
    assert(request & BSDF_REFLECTION); assert(request & BSDF_DIFFUSE); (void)request;
    out_w_gen = Vector(cosine_hemisphere_sample(uvc.x, uvc.y));
    out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
    out_flags = BSDF_REFLECTION | BSDF_DIFFUSE;
    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    return this->albedo * INV_PI;
  }

  float LambertBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector&,
      BxdfFlags request) const 
  {
    assert(request & BSDF_REFLECTION); assert(request & BSDF_DIFFUSE); (void)request;
    return cosine_hemisphere_pdf(w_gen.v.z);
  }

  void LambertMaterial::add_bxdfs(const DiffGeom& geom,
      Spectrum scale, Bsdf& bsdf) const 
  {
    Spectrum albedo = this->albedo->evaluate(geom);
    if(!albedo.is_black()) {
      bsdf.add(std::make_unique<LambertBrdf>(albedo * scale));
    }
  }
}

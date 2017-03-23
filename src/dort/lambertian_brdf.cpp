#include "dort/lambertian_brdf.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/rng.hpp"

namespace dort {
  Spectrum LambertianBrdf::eval_f(const Vector& wi, const Vector& wo) const {
    if(Bsdf::same_hemisphere(wo, wi)) {
      return this->reflectance * INV_PI;
    } else {
      return Spectrum(0.f);
    }
  }

  Spectrum LambertianBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
    out_dir_pdf = cosine_hemisphere_pdf(out_w_gen.v.z);
    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    return this->reflectance * INV_PI;
  }

  float LambertianBrdf::symmetric_f_pdf(const Vector& w_gen,
      const Vector& w_fix) const
  {
    if(!Bsdf::same_hemisphere(w_gen, w_fix)) {
      return 0.f;
    }
    return cosine_hemisphere_pdf(w_gen.v.z);
  }
}

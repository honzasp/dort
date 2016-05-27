#include "dort/lambertian_brdf.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/rng.hpp"

namespace dort {
  Spectrum LambertianBrdf::f(const Vector& wo, const Vector& wi) const {
    if(Bsdf::same_hemisphere(wo, wi)) {
      return this->reflectance * INV_PI;
    } else {
      return Spectrum(0.f);
    }
  }

  Spectrum LambertianBrdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    out_wi = Vector(cosine_hemisphere_sample(u1, u2));
    out_pdf = cosine_hemisphere_pdf(out_wi.v.z);
    if(wo.v.z < 0.f) {
      out_wi.v.z = -out_wi.v.z;
    }
    return this->f(wo, out_wi);
  }

  float LambertianBrdf::f_pdf(const Vector& wo, const Vector& wi) const {
    if(Bsdf::same_hemisphere(wo, wi)) {
      return cosine_hemisphere_pdf(wi.v.z);
    } else {
      return 0.f;
    }
  }
}

#include "dort/monte_carlo.hpp"
#include "dort/phong_brdf.hpp"

namespace dort {
  PhongBrdf::PhongBrdf(const Spectrum& k_diffuse,
      const Spectrum& k_glossy, float exponent):
    Bxdf(BSDF_REFLECTION | BSDF_GLOSSY),
    k_diffuse(k_diffuse), k_glossy(k_glossy), exponent(exponent)
  { 
    float diffuse_weight = this->k_diffuse.average();
    float glossy_weight = this->k_glossy.average();
    this->glossy_pdf = glossy_weight / (diffuse_weight + glossy_weight);
  }

  Spectrum PhongBrdf::f(const Vector& wo, const Vector& wi) const {
    if(!Bsdf::same_hemisphere(wo, wi)) {
      return Spectrum(0.f);
    }

    Spectrum diffuse = this->k_diffuse * INV_PI;

    float cos_alpha = dot(Vector(-wo.v.x, -wo.v.y, wo.v.z), wi);
    if(cos_alpha < 0.f) {
      return diffuse;
    }

    Spectrum glossy = this->k_glossy * (this->exponent + 2.f) * INV_TWO_PI
      * pow(cos_alpha, this->exponent);
    return diffuse + glossy;
  }

  Spectrum PhongBrdf::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    if(u1 < this->glossy_pdf) {
      u1 = u1 / this->glossy_pdf;
      Vector refl_wo = Vector(-wo.v.x, -wo.v.y, wo.v.z);
      Vector refl_s, refl_t;
      coordinate_system(refl_wo, refl_s, refl_t);

      Vec3 relative_wi = power_cosine_hemisphere_sample(u1, u2, this->exponent);
      out_wi = refl_wo * relative_wi.z + refl_s * relative_wi.x + refl_t * relative_wi.y;
      out_pdf = this->glossy_pdf * power_cosine_hemisphere_pdf(
          relative_wi.z, this->exponent);
    } else {
      u1 = (u1 - this->glossy_pdf) / (1.f - this->glossy_pdf);
      out_wi = Vector(cosine_hemisphere_sample(u1, u2));
      out_pdf = (1.f - this->glossy_pdf) * cosine_hemisphere_pdf(out_wi.v.z);
    }

    if(wo.v.z < 0.f) {
      out_wi.v.z = -out_wi.v.z;
    }
    return this->f(wo, out_wi);
  }

  float PhongBrdf::f_pdf(const Vector& wo, const Vector& wi) const {
    if(!Bsdf::same_hemisphere(wo, wi)) {
      return 0.f;
    }
    float cos_alpha = abs_dot(Vector(-wo.v.x, -wo.v.y, wo.v.z), wi);
    return lerp(this->glossy_pdf,
      cosine_hemisphere_pdf(wi.v.z),
      power_cosine_hemisphere_pdf(cos_alpha, this->exponent));
  }
}

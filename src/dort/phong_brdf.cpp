#include "dort/monte_carlo.hpp"
#include "dort/phong_brdf.hpp"

namespace dort {
  PhongBrdf::PhongBrdf(const Spectrum& k_diffuse,
      const Spectrum& k_glossy, float exponent):
    SymmetricBxdf(BSDF_REFLECTION | BSDF_GLOSSY),
    k_diffuse(k_diffuse), k_glossy(k_glossy), exponent(exponent)
  { 
    float diffuse_weight = this->k_diffuse.average();
    float glossy_weight = this->k_glossy.average();
    this->glossy_pdf = glossy_weight / (diffuse_weight + glossy_weight);
  }

  Spectrum PhongBrdf::eval_f(const Vector& wi_light, const Vector& wo_camera) const {
    if(!Bsdf::same_hemisphere(wo_camera, wi_light)) {
      return Spectrum(0.f);
    }

    Spectrum diffuse = this->k_diffuse * INV_PI;
    Vector wo_reflect(-wo_camera.v.x, -wo_camera.v.y, wo_camera.v.z);
    float cos_alpha = dot(wo_reflect, wi_light);
    if(cos_alpha < 0.f) {
      return diffuse;
    }

    Spectrum glossy = this->k_glossy * (this->exponent + 2.f) * INV_TWO_PI
      * pow(cos_alpha, this->exponent);
    return diffuse + glossy;
  }

  Spectrum PhongBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    if(uv.x < this->glossy_pdf) {
      uv.x = uv.x / this->glossy_pdf;
      Vector refl_w = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
      Vector refl_s, refl_t;
      coordinate_system(refl_w, refl_s, refl_t);

      Vec3 relative_wi = power_cosine_hemisphere_sample(uv.x, uv.y, this->exponent);
      out_w_gen = refl_w * relative_wi.z 
        + refl_s * relative_wi.x + refl_t * relative_wi.y;
      out_dir_pdf = this->glossy_pdf * power_cosine_hemisphere_pdf(
          relative_wi.z, this->exponent);
    } else {
      uv.x = (uv.x - this->glossy_pdf) / (1.f - this->glossy_pdf);
      out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
      out_dir_pdf = (1.f - this->glossy_pdf) * cosine_hemisphere_pdf(out_w_gen.v.z);
    }

    if(w_fix.v.z < 0.f) {
      out_w_gen.v.z = -out_w_gen.v.z;
    }
    return this->eval_f(w_fix, out_w_gen);
  }

  float PhongBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector& w_fix) const {
    if(!Bsdf::same_hemisphere(w_gen, w_fix)) {
      return 0.f;
    }
    Vector w_reflect(-w_gen.v.x, -w_gen.v.y, w_gen.v.z);
    float cos_alpha = abs_dot(w_reflect, w_fix);
    return lerp(this->glossy_pdf,
      cosine_hemisphere_pdf(w_fix.v.z),
      power_cosine_hemisphere_pdf(cos_alpha, this->exponent));
  }
}

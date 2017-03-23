#include "dort/monte_carlo.hpp"
#include "dort/phong_brdf.hpp"

namespace dort {
  PhongBrdf::PhongBrdf(const Spectrum& k_diffuse,
      const Spectrum& k_glossy, float exponent):
    SymmetricBxdf(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY),
    k_diffuse(k_diffuse), k_glossy(k_glossy), exponent(exponent)
  { 
    float diffuse_weight = this->k_diffuse.average();
    float glossy_weight = this->k_glossy.average();
    float sum = diffuse_weight + glossy_weight;
    if(sum != 0.f) {
      this->pick_glossy_pdf = glossy_weight / sum;
    } else {
      this->pick_glossy_pdf = 0.f;
    }
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

    Spectrum glossy = (this->exponent + 2.f) * INV_TWO_PI
      * pow(cos_alpha, this->exponent) * this->k_glossy;
    return diffuse + glossy;
  }

  Spectrum PhongBrdf::sample_symmetric_f(const Vector& w_fix,
      Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const
  {
    if(uv.x < this->pick_glossy_pdf) {
      uv.x = uv.x / this->pick_glossy_pdf;
      Vector refl_w = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
      Vector refl_s, refl_t;
      coordinate_system(refl_w, refl_s, refl_t);

      Vec3 relative_wi = power_cosine_hemisphere_sample(uv.x, uv.y, this->exponent);
      out_w_gen = refl_w * relative_wi.z 
        + refl_s * relative_wi.x + refl_t * relative_wi.y;
    } else {
      uv.x = (uv.x - this->pick_glossy_pdf) / (1.f - this->pick_glossy_pdf);
      out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
    }

    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    out_dir_pdf = this->symmetric_f_pdf(out_w_gen, w_fix);
    return this->eval_f(w_fix, out_w_gen);
  }

  float PhongBrdf::symmetric_f_pdf(const Vector& w_gen, const Vector& w_fix) const {
    if(!Bsdf::same_hemisphere(w_gen, w_fix)) {
      return 0.f;
    }
    Vector w_reflect(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    float cos_alpha = abs_dot(w_reflect, w_gen);
    float diffuse_pdf = cosine_hemisphere_pdf(w_gen.v.z);
    float glossy_pdf = power_cosine_hemisphere_pdf(cos_alpha, this->exponent);
    return lerp(this->pick_glossy_pdf, glossy_pdf, diffuse_pdf);
  }
}

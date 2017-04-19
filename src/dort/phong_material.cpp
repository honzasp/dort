#include "dort/bsdf.hpp"
#include "dort/lambert_material.hpp"
#include "dort/phong_material.hpp"

namespace dort {
  PhongBrdf::PhongBrdf(const Spectrum& diffuse_albedo,
      const Spectrum& glossy_albedo, float exponent):
    SymmetricBxdf(BSDF_REFLECTION | BSDF_DIFFUSE | BSDF_GLOSSY),
    diffuse_albedo(diffuse_albedo), glossy_albedo(glossy_albedo), exponent(exponent)
  { 
    float diffuse_weight = this->diffuse_albedo.average();
    float glossy_weight = this->glossy_albedo.average();
    float sum = diffuse_weight + glossy_weight;
    if(sum != 0.f) {
      this->glossy_pdf = glossy_weight / sum;
    } else {
      this->glossy_pdf = 0.f;
    }
  }

  Spectrum PhongBrdf::eval_f(const Vector& wi_light,
      const Vector& wo_camera, BxdfFlags flags) const 
  {
    assert(flags & BSDF_REFLECTION);
    bool sample_diffuse = this->glossy_pdf < 1.f && (flags & BSDF_DIFFUSE);
    bool sample_glossy = this->glossy_pdf > 0.f && (flags & BSDF_GLOSSY);

    Spectrum diffuse = this->diffuse_albedo * INV_PI;
    if(!sample_glossy) { return diffuse; }

    Vector wo_reflect(-wo_camera.v.x, -wo_camera.v.y, wo_camera.v.z);
    float cos_alpha = dot(wo_reflect, wi_light);

    Spectrum glossy = cos_alpha < 0.f ? Spectrum(0.f)
      : (this->exponent + 2.f) * INV_TWO_PI 
        * pow(cos_alpha, this->exponent) * this->glossy_albedo;
    return sample_diffuse ? diffuse + glossy : glossy;
  }

  Spectrum PhongBrdf::sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags, Vec2 uv) const
  {
    assert(request & BSDF_REFLECTION);
    bool sample_diffuse = request & BSDF_DIFFUSE;
    bool sample_glossy = request & BSDF_GLOSSY;
    assert(sample_diffuse || sample_glossy);

    bool glossy;
    if(sample_glossy && sample_diffuse) {
      if((glossy = uv.x < this->glossy_pdf)) {
        uv.x = uv.x / this->glossy_pdf;
      } else {
        uv.x = (uv.x - this->glossy_pdf) / (1.f - this->glossy_pdf);
      }
    } else {
      glossy = sample_glossy;
    }

    if(glossy) {
      Vector refl_w = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
      Vector refl_s, refl_t;
      coordinate_system(refl_w, refl_s, refl_t);

      Vec3 relative_wi = power_cosine_hemisphere_sample(uv.x, uv.y, this->exponent);
      out_w_gen = refl_w * relative_wi.z 
        + refl_s * relative_wi.x + refl_t * relative_wi.y;
      out_flags = BSDF_REFLECTION | BSDF_GLOSSY;
    } else {
      out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
      out_flags = BSDF_REFLECTION | BSDF_DIFFUSE;
    }

    out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
    out_dir_pdf = this->symmetric_f_pdf(out_w_gen, w_fix, request);
    return this->eval_f(w_fix, out_w_gen, request);
  }

  float PhongBrdf::symmetric_f_pdf(const Vector& w_gen,
      const Vector& w_fix, BxdfFlags request) const 
  {
    Vector w_reflect(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
    float cos_alpha = abs_dot(w_reflect, w_gen);
    float diffuse_pdf = cosine_hemisphere_pdf(w_gen.v.z);
    float glossy_pdf = power_cosine_hemisphere_pdf(cos_alpha, this->exponent);

    if((request & BSDF_GLOSSY) && (request & BSDF_DIFFUSE)) {
      return lerp(this->glossy_pdf, glossy_pdf, diffuse_pdf);
    } else if(request & BSDF_GLOSSY) {
      return glossy_pdf;
    } else if(request & BSDF_DIFFUSE) {
      return diffuse_pdf;
    } else {
      return 0.f;
    }
  }

  void PhongMaterial::add_bxdfs(const DiffGeom& geom, Spectrum scale, Bsdf& bsdf) const {
    Spectrum diffuse = this->diffuse_albedo->evaluate(geom);
    Spectrum glossy = this->glossy_albedo->evaluate(geom);
    if((diffuse + glossy).max() > 1.f) {
      float darken = 1.f / (diffuse + glossy).max();
      diffuse *= darken;
      glossy *= darken;
    }
    float exponent = this->exponent->evaluate(geom);

    if(!glossy.is_black()) {
      bsdf.add(std::make_unique<PhongBrdf>(diffuse * scale, glossy * scale, exponent));
    } else if(!diffuse.is_black()) {
      bsdf.add(std::make_unique<LambertBrdf>(diffuse * scale));
    }
  }
}

#pragma once
#include "dort/bsdf.hpp"
#include "dort/lambert_material.hpp"
#include "dort/material.hpp"
#include "dort/texture.hpp"

namespace dort {
  class PhongBrdf final: public SymmetricBxdf {
    Spectrum diffuse_albedo;
    Spectrum glossy_albedo;
    float exponent;
    float glossy_pdf;
  public:
    PhongBrdf(const Spectrum& diffuse_albedo,
        const Spectrum& glossy_albedo, float exponent):
      SymmetricBxdf(BSDF_REFLECTION | BSDF_GLOSSY),
      diffuse_albedo(diffuse_albedo), glossy_albedo(glossy_albedo), exponent(exponent)
    { 
      float diffuse_weight = this->diffuse_albedo.average();
      float glossy_weight = this->glossy_albedo.average();
      float sum = diffuse_weight + glossy_weight;
      if(sum != 0.f) {
        this->glossy_pdf= glossy_weight / sum;
      } else {
        this->glossy_pdf= 0.f;
      }
    }

    virtual Spectrum eval_f(const Vector& wi_light,
        const Vector& wo_camera) const override final
    {
      if(!Bsdf::same_hemisphere(wo_camera, wi_light)) {
        return Spectrum(0.f);
      }

      Spectrum diffuse = this->diffuse_albedo * INV_PI;
      Vector wo_reflect(-wo_camera.v.x, -wo_camera.v.y, wo_camera.v.z);
      float cos_alpha = dot(wo_reflect, wi_light);
      if(cos_alpha < 0.f) {
        return diffuse;
      }

      Spectrum glossy = (this->exponent + 2.f) * INV_TWO_PI
        * pow(cos_alpha, this->exponent) * this->glossy_albedo;
      return diffuse + glossy;
    }

    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const override final
    {
      if(uv.x < this->glossy_pdf) {
        uv.x = uv.x / this->glossy_pdf;
        Vector refl_w = Vector(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
        Vector refl_s, refl_t;
        coordinate_system(refl_w, refl_s, refl_t);

        Vec3 relative_wi = power_cosine_hemisphere_sample(uv.x, uv.y, this->exponent);
        out_w_gen = refl_w * relative_wi.z 
          + refl_s * relative_wi.x + refl_t * relative_wi.y;
      } else {
        uv.x = (uv.x - this->glossy_pdf) / (1.f - this->glossy_pdf);
        out_w_gen = Vector(cosine_hemisphere_sample(uv.x, uv.y));
      }

      out_w_gen.v.z = copysign(out_w_gen.v.z, w_fix.v.z);
      out_dir_pdf = this->symmetric_f_pdf(out_w_gen, w_fix);
      return this->eval_f(w_fix, out_w_gen);
    }

    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const override final
    {
      if(!Bsdf::same_hemisphere(w_gen, w_fix)) {
        return 0.f;
      }
      Vector w_reflect(-w_fix.v.x, -w_fix.v.y, w_fix.v.z);
      float cos_alpha = abs_dot(w_reflect, w_gen);
      float diffuse_pdf = cosine_hemisphere_pdf(w_gen.v.z);
      float glossy_pdf = power_cosine_hemisphere_pdf(cos_alpha, this->exponent);
      return lerp(this->glossy_pdf, glossy_pdf, diffuse_pdf);
    }
  };

  class PhongMaterial final: public Material {
    std::shared_ptr<TextureGeom<Spectrum>> diffuse_albedo;
    std::shared_ptr<TextureGeom<Spectrum>> glossy_albedo;
    std::shared_ptr<TextureGeom<float>> exponent;
  public:
    PhongMaterial(
        std::shared_ptr<TextureGeom<Spectrum>> diffuse_albedo,
        std::shared_ptr<TextureGeom<Spectrum>> glossy_albedo,
        std::shared_ptr<TextureGeom<float>> exponent):
      diffuse_albedo(diffuse_albedo),
      glossy_albedo(glossy_albedo),
      exponent(exponent)
    { }

    virtual void add_bxdfs(const DiffGeom& geom,
        Spectrum scale, Bsdf& bsdf) const override final
    {
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
  };
}

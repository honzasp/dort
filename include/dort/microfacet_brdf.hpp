#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  template<class D, class F, class G>
  class MicrofacetBrdf final: public Bxdf {
    Spectrum reflectance;
    D distrib;
    F fresnel;
    G geom;
  public:
    MicrofacetBrdf(const Spectrum& reflectance, D distrib, F fresnel, G geom):
      Bxdf(BSDF_REFLECTION | BSDF_GLOSSY),
      reflectance(reflectance),
      distrib(std::move(distrib)),
      fresnel(std::move(fresnel)),
      geom(std::move(geom)) 
    { }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const override final;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const override final;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const override final;
  private:
    Spectrum f(const Vector& wo, const Vector& wi, const Vector& m) const;
    float get_eta_o(const Vector& wo) const;
  };
}

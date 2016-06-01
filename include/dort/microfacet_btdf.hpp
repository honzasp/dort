#pragma once
#include "dort/bsdf.hpp"

namespace dort {
  template<class D, class F, class G>
  class MicrofacetBtdf final: public Bxdf {
    Spectrum transmittance;
    D distrib;
    F fresnel;
    G geom;
  public:
    MicrofacetBtdf(const Spectrum& transmittance, D distrib, F fresnel, G geom):
      Bxdf(BSDF_TRANSMISSION | BSDF_GLOSSY),
      transmittance(transmittance),
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

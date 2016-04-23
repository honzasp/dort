#include "dort/fresnel.hpp"
#include "dort/microfacet_brdf.hpp"
#include "dort/microfacet_distrib.hpp"

namespace dort {
  template<class D, class F, class G>
  Spectrum MicrofacetBrdf<D, F, G>::f(const Vector& wo, const Vector& wi) const {
    return this->f(wo, wi, normalize(wo + wi));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBrdf<D, F, G>::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    float m_pdf;
    Vector m = this->distrib.sample_m(u1, u2, m_pdf);
    Vector wi = 2.f * dot(wo, m) * m - wo;
    out_wi = wi;
    out_pdf = m_pdf / (4.f * abs_dot(wi, m));
    return this->f(wo, wi, m);
  }

  template<class D, class F, class G>
  float MicrofacetBrdf<D, F, G>::f_pdf(const Vector& wo, const Vector& wi) const {
    Vector m = normalize(wo + wi);
    float m_pdf = this->distrib.m_pdf(m);
    return m_pdf / (4.f * abs_dot(wi, m));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBrdf<D, F, G>::f(const Vector& wo,
      const Vector& wi, const Vector& m) const 
  {
    float cos_o = Bsdf::cos_theta(wo);
    float cos_i = Bsdf::cos_theta(wi);
    // TODO: is this correct??
    float eta_o = this->get_eta_o(wo);
    Vector transmit_m = -normalize(eta_o * wo + wi);
    float fresnel = this->fresnel.reflectance(dot(wi, transmit_m), dot(wo, transmit_m));
    float geom = this->geom.g(wo, wi, m);
    float dis = this->distrib.d(m);
    return this->reflectance * (0.25f * fresnel * geom * dis / (cos_o * cos_i));
  }

  template class MicrofacetBrdf<
    BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>;
  template class MicrofacetBrdf<
    BeckmannD, FresnelConductor, SmithG<BeckmannApproxG1>>;
}

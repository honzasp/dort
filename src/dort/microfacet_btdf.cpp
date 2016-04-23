#include "dort/fresnel.hpp"
#include "dort/microfacet_btdf.hpp"
#include "dort/microfacet_distrib.hpp"

namespace dort {
  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::f(const Vector& wo, const Vector& wi) const {
    float eta_o = this->get_eta_o(wo);
    return this->f(wo, wi, -normalize(eta_o * wo + wi));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    float eta_o = this->get_eta_o(wo);
    float m_pdf;
    Vector m = this->distrib.sample_m(u1, u2, m_pdf);

    float c = dot(wo, m);
    Vector wi = m * (eta_o * c - copysign(sqrt(abs(1.f + eta_o * (c*c - 1.f))),
          dot(m, wo))) - eta_o * wo;
    out_wi = wi;
    out_pdf = m_pdf * abs_dot(wi, m) / square(dot(wi, m) + eta_o * dot(wo, m));
    return this->f(wo, out_wi, m);
  }

  template<class D, class F, class G>
  float MicrofacetBtdf<D, F, G>::f_pdf(const Vector& wo, const Vector& wi) const {
    float eta_o = this->get_eta_o(wo);
    Vector m = -normalize(eta_o * wo + wi);
    float m_pdf = this->distrib.m_pdf(m);
    return m_pdf * abs_dot(wi, m) / square(dot(wi, m) + eta_o * dot(wo, m));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::f(const Vector& wo,
      const Vector& wi, const Vector& m) const 
  {
    float eta_o = this->get_eta_o(wo);
    float dot_i_m = dot(wi, m);
    float dot_o_m = dot(wo, m);
    float dot_i_n = Bsdf::cos_theta(wi);
    float dot_o_n = Bsdf::cos_theta(wo);

    float fresnel = this->fresnel.reflectance(dot_i_m, dot_o_m);
    float geom = this->geom.g(wo, wi, m);
    float dis = this->distrib.d(m);

    if(fresnel == 1.f || geom == 0.f || dis == 0.f) {
      return Spectrum(0.f);
    }

    float tmp_1 = abs(dot_i_m * dot_o_m) * square(eta_o) *
      (1.f - fresnel) * geom * dis;
    float tmp_2 = abs(dot_i_n * dot_o_n) * square(dot_i_m + eta_o * dot_o_m);
    return this->transmittance * (tmp_1 / tmp_2);
  }

  template<class D, class F, class G>
  float MicrofacetBtdf<D, F, G>::get_eta_o(const Vector& wo) const {
    return Bsdf::cos_theta(wo) > 0.f ? this->fresnel.inv_eta : this->fresnel.eta;
  }

  template class MicrofacetBtdf<
    BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>;
  template class MicrofacetBtdf<
    BeckmannD, FresnelConductor, SmithG<BeckmannApproxG1>>;
}

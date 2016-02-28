#include "dort/microfacet_btdf.hpp"

namespace dort {
  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::f(const Vector& wo, const Vector& wi) const {
    float eta_t, eta_i;
    this->get_eta(wo, eta_t, eta_i);
    return this->f(wo, wi, -normalize(eta_t * wo + eta_i * wi));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::sample_f(const Vector& wo, Vector& out_wi,
      float& out_pdf, float u1, float u2) const
  {
    float eta_t, eta_i;
    this->get_eta(wo, eta_t, eta_i);

    float m_pdf;
    Vector m = this->distrib.sample_m(u1, u2, m_pdf);

    float c = dot(wo, m);
    float eta = eta_t / eta_i;
    Vector wi = (eta * c - copysign(sqrt(1.f + eta * (c * c - 1.f)),
          Bsdf::cos_theta(wo))) * m - eta * wo;

    out_wi = wi;
    out_pdf = m_pdf * square(eta_t) * dot(wi, m) /
      square(eta_t * dot(wo, m) + eta_i * dot(wi, m));
    return this->f(wo, wi, m);
  }

  template<class D, class F, class G>
  float MicrofacetBtdf<D, F, G>::f_pdf(const Vector& wo, const Vector& wi) const {
    float eta_t, eta_i;
    this->get_eta(wo, eta_t, eta_i);
    Vector m = -normalize(eta_t * wo + eta_i * wi);
    float m_pdf = this->distrib.m_pdf(m);
    return m_pdf * square(eta_t) * dot(wi, m) /
      square(eta_t * dot(wo, m) + eta_i * dot(wi, m));
  }

  template<class D, class F, class G>
  Spectrum MicrofacetBtdf<D, F, G>::f(const Vector& wo,
      const Vector& wi, const Vector& m) const 
  {
    float cos_o = Bsdf::cos_theta(wo);
    float cos_i = Bsdf::cos_theta(wi);
    float fresnel = this->fresnel.reflectance(dot(wi, m));
    float geom = this->geom.g(wo, wi, m);
    float dis = this->distrib.d(m);
    return this->reflectance * (0.25f * fresnel * geom * dis / (cos_o * cos_i));
  }

  template<class D, class F, class G>
  void MicrofacetBtdf<D, F, G>::get_eta(const Vector& wo,
      float& out_eta_t, float& out_eta_i) 
  {
    if(Bsdf::cos_theta(wo) < 0.f) {
      out_eta_i = this->fresnel.eta_t();
      out_eta_t = this->fresnel.eta_i();
    } else {
      out_eta_i = this->fresnel.eta_i();
      out_eta_t = this->fresnel.eta_t();
    }
  }
}

#include "dort/fresnel.hpp"
#include "dort/microfacet_brdf.hpp"
#include "dort/microfacet_btdf.hpp"
#include "dort/microfacet_distrib.hpp"
#include "dort/plastic_material.hpp"
#include "dort/rough_glass_material.hpp"
#include "dort/specular.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> RoughGlassMaterial::get_bsdf(
      const DiffGeom& diff_geom) const 
  {
    Spectrum reflect = this->reflectance->evaluate(diff_geom);
    Spectrum transmit = this->transmittance->evaluate(diff_geom);
    float roughness = max(0.f, this->roughness->evaluate(diff_geom));
    float eta = this->eta->evaluate(diff_geom);
    float alpha_b = roughness;

    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    if(alpha_b > 1e-3f) {
      if(!reflect.is_black()) {
        bsdf->add(std::make_unique<MicrofacetBrdf<
            BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>>(
          reflect, BeckmannD(alpha_b), FresnelDielectric(1.f, eta),
          SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
      }
      if(!transmit.is_black()) {
        bsdf->add(std::make_unique<MicrofacetBtdf<
            BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>>(
          transmit, BeckmannD(alpha_b), FresnelDielectric(1.f, eta),
          SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
      }
    } else {
      if(!reflect.is_black()) {
        bsdf->add(std::make_unique<SpecularBrdf<FresnelDielectric>>(
              reflect, FresnelDielectric(1.f, eta)));
      }
      if(!transmit.is_black()) {
        bsdf->add(std::make_unique<SpecularBtdf>(
              transmit, FresnelDielectric(1.f, eta)));
      }
    }
    return bsdf;
  }
}

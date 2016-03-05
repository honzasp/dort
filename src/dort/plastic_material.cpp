#include "dort/fresnel.hpp"
#include "dort/lambertian_brdf.hpp"
#include "dort/microfacet_brdf.hpp"
#include "dort/microfacet_distrib.hpp"
#include "dort/plastic_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> PlasticMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    Spectrum diffuse = this->diffuse->evaluate(diff_geom);
    Spectrum reflection = this->reflection->evaluate(diff_geom);
    float roughness = max(1e-3f, this->roughness->evaluate(diff_geom));
    float eta = this->eta->evaluate(diff_geom);

    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    if(!diffuse.is_black()) {
      bsdf->add(std::make_unique<LambertianBrdf>(diffuse));
    }
    if(!reflection.is_black()) {
      float alpha_b = roughness;
      bsdf->add(std::make_unique<MicrofacetBrdf<
          BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>>(
        reflection,
        BeckmannD(alpha_b),
        FresnelDielectric(1.f, eta),
        SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
    }
    return bsdf;
  }
}

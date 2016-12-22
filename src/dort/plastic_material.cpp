#include "dort/fresnel.hpp"
#include "dort/lambertian_brdf.hpp"
#include "dort/microfacet.hpp"
#include "dort/microfacet_distrib.hpp"
#include "dort/plastic_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> PlasticMaterial::get_bsdf(
      const DiffGeom& shading_geom, const Normal& nn_geom) const 
  {
    Spectrum diffuse = this->diffuse->evaluate(shading_geom);
    //Spectrum reflection = this->reflection->evaluate(shading_geom);
    //float roughness = max(1e-3f, this->roughness->evaluate(shading_geom));
    //float eta = this->eta->evaluate(shading_geom);

    auto bsdf = std::make_unique<Bsdf>(shading_geom, nn_geom);
    if(!diffuse.is_black()) {
      bsdf->add(std::make_unique<LambertianBrdf>(diffuse));
    }
    /*
    if(!reflection.is_black()) {
      float alpha_b = roughness;
      bsdf->add(std::make_unique<MicrofacetBrdf<
          BeckmannD, FresnelDielectric, SmithG<BeckmannApproxG1>>>(
        reflection,
        BeckmannD(alpha_b),
        FresnelDielectric(eta),
        SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
    }
    */
    return bsdf;
  }
}

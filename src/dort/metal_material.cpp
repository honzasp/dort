#include "dort/fresnel.hpp"
#include "dort/metal_material.hpp"
#include "dort/microfacet.hpp"
#include "dort/microfacet_distrib.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MetalMaterial::get_bsdf(
      const DiffGeom& shading_geom, const Normal& nn_geom) const 
  {
    //Spectrum reflection = this->reflection->evaluate(shading_geom);
    //float roughness = max(1e-3f, this->roughness->evaluate(shading_geom));
    //float eta = this->eta->evaluate(shading_geom);
    //float k = this->k->evaluate(shading_geom);

    auto bsdf = std::make_unique<Bsdf>(shading_geom, nn_geom);
    //float alpha_b = roughness;
    /*
    bsdf->add(std::make_unique<MicrofacetBrdf<
        BeckmannD, FresnelConductor, SmithG<BeckmannApproxG1>>>(
      reflection,
      BeckmannD(alpha_b),
      FresnelConductor(eta, k),
      SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
      */
    return bsdf;
  }
}

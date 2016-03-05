#include "dort/fresnel.hpp"
#include "dort/metal_material.hpp"
#include "dort/microfacet_brdf.hpp"
#include "dort/microfacet_distrib.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MetalMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    Spectrum reflection = this->reflection->evaluate(diff_geom);
    float roughness = max(1e-3f, this->roughness->evaluate(diff_geom));
    float eta = this->eta->evaluate(diff_geom);
    float k = this->k->evaluate(diff_geom);

    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    float alpha_b = roughness;
    bsdf->add(std::make_unique<MicrofacetBrdf<
        BeckmannD, FresnelConductor, SmithG<BeckmannApproxG1>>>(
      reflection,
      BeckmannD(alpha_b),
      FresnelConductor(eta, k),
      SmithG<BeckmannApproxG1>(BeckmannApproxG1(alpha_b))));
    return bsdf;
  }
}

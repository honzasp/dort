#include "dort/lambertian_brdf.hpp"
#include "dort/plastic_material.hpp"
#include "dort/texture.hpp"
#include "dort/torrance_sparrow_brdf.hpp"

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
      bsdf->add(std::make_unique<TorranceSparrowBrdf>(reflection,
            std::make_unique<BlinnMicrofacetDistribution>(1.f / roughness),
            std::make_unique<FresnelDielectric>(eta, 1.f)));
    }
    return bsdf;
  }
}

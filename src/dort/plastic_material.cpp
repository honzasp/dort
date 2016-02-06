#include "dort/lambertian_brdf.hpp"
#include "dort/plastic_material.hpp"
#include "dort/torrance_sparrow_brdf.hpp"

namespace dort {
  std::unique_ptr<Bsdf> PlasticMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    bsdf->add(std::make_unique<LambertianBrdf>(this->diffuse));
    bsdf->add(std::make_unique<TorranceSparrowBrdf>(this->reflection,
          std::make_unique<BlinnMicrofacetDistribution>(1.f / this->roughness),
          std::make_unique<FresnelDielectric>(this->eta, 1.f)));
    return bsdf;
  }
}

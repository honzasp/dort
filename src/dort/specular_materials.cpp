#include "dort/specular.hpp"
#include "dort/specular_materials.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MirrorMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    bsdf->add(std::make_unique<SpecularBrdf>(this->reflectance,
          std::make_unique<FresnelConstant>(1.f)));
    return bsdf;
  }

  std::unique_ptr<Bsdf> GlassMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    bsdf->add(std::make_unique<SpecularBrdf>(this->reflectance,
          std::make_unique<FresnelDielectric>(1.f, this->eta)));
    bsdf->add(std::make_unique<SpecularBtdf>(this->transmittance,
          FresnelDielectric(1.f, this->eta)));
    return bsdf;
  }
}

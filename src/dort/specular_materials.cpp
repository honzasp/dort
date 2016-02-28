#include "dort/fresnel.hpp"
#include "dort/specular.hpp"
#include "dort/specular_materials.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MirrorMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    Spectrum reflect = this->reflectance->evaluate(diff_geom);
    bsdf->add(std::make_unique<SpecularBrdf<FresnelConstant>>(
          reflect, FresnelConstant(1.f, 1.f)));
    return bsdf;
  }

  std::unique_ptr<Bsdf> GlassMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    Spectrum reflect = this->reflectance->evaluate(diff_geom);
    Spectrum transmit = this->transmittance->evaluate(diff_geom);
    float eta = this->eta->evaluate(diff_geom);

    bsdf->add(std::make_unique<SpecularBrdf<FresnelDielectric>>(
          reflect, FresnelDielectric(1.f, eta)));
    return bsdf;
  }
}

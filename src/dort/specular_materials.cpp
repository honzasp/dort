#include "dort/fresnel.hpp"
#include "dort/specular.hpp"
#include "dort/specular_materials.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MirrorMaterial::get_bsdf(
      const DiffGeom& shading_geom, const Normal& nn_geom) const 
  {
    auto bsdf = std::make_unique<Bsdf>(shading_geom, nn_geom);
    Spectrum reflect = this->reflectance->evaluate(shading_geom);

    if(!reflect.is_black()) {
      bsdf->add(std::make_unique<MirrorBrdf<FresnelConstant>>(
            reflect, FresnelConstant(1.f)));
    }
    return bsdf;
  }

  std::unique_ptr<Bsdf> GlassMaterial::get_bsdf(
      const DiffGeom& shading_geom, const Normal& nn_geom) const 
{
    auto bsdf = std::make_unique<Bsdf>(shading_geom, nn_geom);
    Spectrum reflect = this->reflectance->evaluate(shading_geom);
    Spectrum transmit = this->transmittance->evaluate(shading_geom);
    float eta = this->eta->evaluate(shading_geom);

    if(!reflect.is_black() || !transmit.is_black()) {
      bsdf->add(std::make_unique<FresnelBxdf>(reflect, transmit,
            FresnelDielectric(1.f, eta), 1.f, eta));
    }
    return bsdf;
  }
}

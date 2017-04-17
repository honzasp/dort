#include "dort/bsdf.hpp"
#include "dort/lambertian_brdf.hpp"
#include "dort/phong_brdf.hpp"
#include "dort/phong_material.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> PhongMaterial::get_bsdf(const DiffGeom& geom) const {
    auto bsdf = std::make_unique<Bsdf>(geom);
    Spectrum k_diffuse = this->k_diffuse->evaluate(geom);
    Spectrum k_glossy = this->k_diffuse->evaluate(geom);
    float exponent = this->exponent->evaluate(geom);
    if(!k_glossy.is_black()) {
      bsdf->add(std::make_unique<PhongBrdf>(k_diffuse, k_glossy, exponent));
    } else {
      bsdf->add(std::make_unique<LambertianBrdf>(k_diffuse));
    }
    return bsdf;
  }
}

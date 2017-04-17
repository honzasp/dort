#include "dort/lambertian_brdf.hpp"
#include "dort/matte_material.hpp"
#include "dort/oren_nayar_brdf.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MatteMaterial::get_bsdf(const DiffGeom& geom) const {
    auto bsdf = std::make_unique<Bsdf>(geom);
    float sigma = this->sigma->evaluate(geom);
    Spectrum reflectance = this->reflectance->evaluate(geom);
    if(sigma == 0.f) {
      bsdf->add(std::make_unique<LambertianBrdf>(reflectance));
    } else {
      bsdf->add(std::make_unique<OrenNayarBrdf>(reflectance, sigma));
    }

    return bsdf;
  }
}

#include "dort/lambertian_brdf.hpp"
#include "dort/matte_material.hpp"
#include "dort/oren_nayar_brdf.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MatteMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    float sigma = this->sigma->evaluate(diff_geom);
    Spectrum reflectance = this->reflectance->evaluate(diff_geom);
    if(sigma == 0.f) {
      bsdf->add(std::make_unique<LambertianBrdf>(reflectance));
    } else {
      bsdf->add(std::make_unique<OrenNayarBrdf>(reflectance, sigma));
    }
    return bsdf;
  }
}

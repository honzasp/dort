#include "dort/lambertian_brdf.hpp"
#include "dort/matte_material.hpp"
#include "dort/oren_nayar_brdf.hpp"

namespace dort {
  std::unique_ptr<Bsdf> MatteMaterial::get_bsdf(const DiffGeom& diff_geom) const {
    auto bsdf = std::make_unique<Bsdf>(diff_geom);
    if(this->sigma == 0.f) {
      bsdf->add(std::make_unique<LambertianBrdf>(this->reflectance));
    } else {
      bsdf->add(std::make_unique<OrenNayarBrdf>(this->reflectance, sigma));
    }
    return bsdf;
  }
}

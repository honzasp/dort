#pragma once
#include "dort/dort.hpp"

#include "dort/lambertian_brdf.hpp"

namespace dort {
  class Material {
  public:
    virtual ~Material() { }
    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) const = 0;
  };

  class MatteMaterial: public Material {
    Spectrum reflectance;
  public:
    MatteMaterial(const Spectrum& reflectance):
      reflectance(reflectance)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) const override final
    {
      auto bsdf = std::make_unique<Bsdf>(diff_geom);
      bsdf->add(std::make_unique<LambertianBrdf>(this->reflectance));
      return bsdf;
    }
  };
}

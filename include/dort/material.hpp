#pragma once
#include "dort/dort.hpp"

#include "dort/lambertian_brdf.hpp"
#include "dort/specular.hpp"

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

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };

  class MirrorMaterial: public Material {
    Spectrum reflectance;
  public:
    MirrorMaterial(const Spectrum& reflectance):
      reflectance(reflectance)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) 
      const override final;
  };

  class GlassMaterial: public Material {
    Spectrum reflectance;
    Spectrum transmittance;
    float eta;
  public:
    GlassMaterial(const Spectrum& reflectance,
        const Spectrum& transmittance, float eta):
      reflectance(reflectance),
      transmittance(transmittance),
      eta(eta)
    { }

    virtual std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom)
      const override final;
  };
}

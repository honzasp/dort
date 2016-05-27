#pragma once
#include "dort/light.hpp"

namespace dort {
  class DirectionalLight final: public Light {
    Vector direction;
    Vector s, t;
    Spectrum radiance;
  public:
    DirectionalLight(const Vector& dir, const Spectrum& radiance);
    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pdf,
        LightRaySample sample) const override final;
    virtual Spectrum sample_radiance(const Point& eye, float eye_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
        LightSample sample) const override final;
    virtual float radiance_pdf(const Point& eye, const Vector& wi) const override final;
    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual Spectrum approximate_power(const Scene& scene) const override final;
  };
}

#pragma once
#include "dort/light.hpp"
#include "dort/transform.hpp"

namespace dort {
  class DiffuseLight final: public AreaLight {
    std::shared_ptr<Shape> shape;
    Transform shape_to_world;
    Spectrum radiance;
    float area;
  public:
    DiffuseLight(std::shared_ptr<Shape> shape,
        const Transform& shape_to_world,
        Spectrum radiance,
        uint32_t num_samples);

    virtual Spectrum sample_radiance(const Point& eye, float eye_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
        LightSample sample) const override final;
    virtual float radiance_pdf(const Point& eye, const Vector& wi) const override final;
    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual Spectrum emitted_radiance(const Point& pt,
        const Normal& n, const Vector& wo) const override final;
  };
}

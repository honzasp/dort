#pragma once
#include "dort/light.hpp"
#include "dort/transform.hpp"

namespace dort {
  class DiffuseLight final: public Light {
    std::shared_ptr<Shape> shape;
    Transform shape_to_world;
    Spectrum radiance;
    float area;
  public:
    DiffuseLight(std::shared_ptr<Shape> shape,
        const Transform& shape_to_world, Spectrum radiance);

    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
        LightRaySample sample) const override final;
    virtual Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
        float& out_dir_pdf, ShadowTest& out_shadow,
        LightSample sample) const override final;
    virtual bool sample_point(Point& out_p, float& p_epsilon,
        Normal& out_nn, float& out_pos_pdf, LightSample sample) const override final;
    virtual Spectrum eval_radiance(const Point& pt,
        const Normal& nn, const Point& pivot) const override final;

    virtual float ray_radiance_pdf(const Scene& scene, const Point& origin_gen,
        const Vector& dir_gen, const Normal& nn) const override final;
    virtual float pivot_radiance_pdf(
        const Vector& wi_gen, const Point& pivot_fix) const override final;

    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual Spectrum approximate_power(const Scene& scene) const override final;
  };
}

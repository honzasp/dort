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
        const Transform& shape_to_world,
        Spectrum radiance,
        uint32_t num_samples);

    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
        LightRaySample sample) const override final;
    virtual Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, float& out_dir_pdf, ShadowTest& out_shadow, 
        LightSample sample) const override final;
    virtual bool sample_point(Point& out_p, float& p_epsilon,
        Normal& out_nn, float& out_pos_pdf, LightSample sample) const override final;
    virtual Spectrum eval_radiance(const Point& pt,
        const Normal& nn, const Point& pivot) const override final;

    virtual float ray_dir_radiance_pdf(const Scene& scene, const Vector& wo_gen,
        const Point& origin_fix, const Normal& nn_fix) const override final;
    virtual float ray_orig_radiance_pdf(const Scene& scene,
        const Point& origin_gen) const override final;
    virtual float pivot_radiance_pdf(const Point& pivot,
        const Vector& wi) const override final;

    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual Spectrum approximate_power(const Scene& scene) const override final;
  };
}

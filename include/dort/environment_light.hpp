#pragma once
#include "dort/light.hpp"
#include "dort/piecewise_distrib_2d.hpp"

namespace dort {
  class EnvironmentLight: public Light {
    std::shared_ptr<Image<PixelRgbFloat>> image;
    PiecewiseDistrib2d distrib;
    Vector up, s, t;
    Spectrum average_radiance;
    Spectrum scale;
  public:
    EnvironmentLight(std::shared_ptr<Image<PixelRgbFloat>> image,
        const Spectrum& scale, const Vector& up, const Vector& fwd, uint32_t num_samples);

    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
        LightRaySample sample) const override final;
    virtual Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
        float& out_dir_pdf, ShadowTest& out_shadow,
        LightSample sample) const override final;
    virtual bool sample_point(Point& out_p, float& out_p_epsilon,
        Normal& out_nn, float& out_pos_pdf, LightSample sample) const override final;
    virtual Spectrum eval_radiance(const Point& pt,
        const Normal& nn, const Point& pivot) const override final;
    
    virtual float ray_radiance_pdf(const Scene& scene, const Point& origin_gen,
        const Vector& dir_gen, const Normal& nn) const override final;
    virtual float pivot_radiance_pdf(
        const Vector& wi_gen, const Point& pivot_fix) const override final;

    virtual Spectrum background_radiance(const Ray& ray) const override final;
    virtual Spectrum approximate_power(const Scene& scene) const override final;
  private:
    Vector sample_dir(Vec2 uv, Vec2& out_dir_uv, float& our_dir_pdf) const;
    float dir_pdf(const Vector& dir) const;
    float dir_uv_pdf(Vec2 dir_uv) const;
    Vector dir_uv_to_dir(Vec2 dir_uv, float& out_sin_theta) const;
    Vec2 dir_to_dir_uv(const Vector& dir, float& out_sin_theta) const;
    Spectrum get_radiance(Vec2 dir_uv) const;
  };
}

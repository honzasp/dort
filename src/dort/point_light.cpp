#include "dort/monte_carlo.hpp"
#include "dort/point_light.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum PointLight::sample_ray_radiance(const Scene&, 
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample sample) const
  {
    Vec3 wi = uniform_sphere_sample(sample.uv_dir.x, sample.uv_dir.y);
    out_ray = Ray(this->pt, Vector(wi), 0.f);
    out_nn = Normal(wi);
    out_pos_pdf = 1.f;
    out_dir_pdf = INV_FOUR_PI;
    return this->intensity;
  }

  Spectrum PointLight::sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
      Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
      float& out_dir_pdf, ShadowTest& out_shadow, LightSample sample) const
  {
    (void)sample;
    out_wi = normalize(this->pt - pivot);
    out_shadow.init_point_point(pivot, pivot_epsilon, this->pt, 0.f);
    out_p = this->pt;
    out_nn = -Normal(out_wi);
    out_p_epsilon = 0.f;
    out_dir_pdf = length_squared(this->pt - pivot);
    return this->intensity;
  }

  bool PointLight::sample_point(Point& out_p, float& out_p_epsilon,
      Normal& out_nn, float& out_pos_pdf, LightSample) const
  {
    out_p = this->pt;
    out_p_epsilon = 0.f;
    out_nn = Normal();
    out_pos_pdf = 1.f;
    return true;
  }

  Spectrum PointLight::eval_radiance(const Point&,
      const Normal&, const Point& pivot) const
  {
    return this->intensity / length_squared(this->pt - pivot);
  }

  float PointLight::ray_radiance_pdf(const Scene&, const Point&,
      const Vector&, const Normal&) const
  {
    return 1.f / (4.f * PI);
  }

  float PointLight::pivot_radiance_pdf(const Vector&, const Point& pivot_fix) const {
    return length_squared(this->pt - pivot_fix);
  }

  Spectrum PointLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum PointLight::approximate_power(const Scene&) const {
    return FOUR_PI * this->intensity;
  }
}

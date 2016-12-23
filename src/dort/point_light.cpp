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
    out_dir_pdf = uniform_sphere_pdf();
    return this->intensity;
  }

  Spectrum PointLight::sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
      Vector& out_wi, float& out_dir_pdf, ShadowTest& out_shadow, 
      LightSample) const
  {
    out_wi = normalize(this->pt - pivot);
    out_dir_pdf = 1.f;
    out_shadow.init_point_point(pivot, pivot_epsilon, this->pt, 0.f);
    return this->intensity / length_squared(this->pt - pivot);
  }

  float PointLight::ray_origin_radiance_pdf(const Scene&,
      const Point&, const Vector&) const 
  {
    return 0.f;
  }

  float PointLight::ray_dir_radiance_pdf(const Scene&,
      const Vector&, const Point&, const Normal&) const
  {
    return 0.f;
  }

  float PointLight::pivot_radiance_pdf(const Point&, const Vector&) const {
    return 0.f;
  }

  Spectrum PointLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum PointLight::approximate_power(const Scene&) const {
    return FOUR_PI * this->intensity;
  }
}

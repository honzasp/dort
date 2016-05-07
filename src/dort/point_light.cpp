#include "dort/monte_carlo.hpp"
#include "dort/point_light.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum PointLight::sample_ray_radiance(const Scene&, 
      Ray& out_ray, Normal& out_nn, float& out_pdf,
      LightRaySample sample) const
  {
    Vector wi = uniform_sphere_sample(sample.uv_dir.x, sample.uv_dir.y);
    out_ray = Ray(this->pt, wi, 0.f);
    out_nn = Normal(wi);
    out_pdf = uniform_sphere_pdf();
    return this->intensity;
  }

  Spectrum PointLight::sample_radiance(const Point& eye, float eye_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
      LightSample) const
  {
    out_wi = normalize(this->pt - eye);
    out_pdf = 1.f;
    out_shadow.init_point_point(eye, eye_epsilon, this->pt, 0.f);
    return this->intensity / length_squared(this->pt - eye);
  }

  float PointLight::radiance_pdf(const Point&, const Vector&) const {
    return 0.f;
  }

  Spectrum PointLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum PointLight::approximate_power(const Scene&) const {
    return FOUR_PI * this->intensity;
  }
}

#include "dort/infinite_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  Spectrum InfiniteLight::sample_radiance(const Point& eye, float eye_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    out_wi = uniform_sphere_sample(sample.uv_pos.x, sample.uv_pos.y);
    out_pdf = uniform_sphere_pdf();
    out_shadow.init_point_dir(eye, eye_epsilon, out_wi);
    return this->radiance;
  }

  float InfiniteLight::radiance_pdf(const Point&, const Vector&) const {
    return uniform_sphere_pdf();
  }

  Spectrum InfiniteLight::background_radiance(const Ray&) const {
    return this->radiance;
  }

  bool InfiniteLight::is_delta() const {
    return false;
  }
}

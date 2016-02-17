#include "dort/point_light.hpp"
#include "dort/primitive.hpp"

namespace dort {
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

  bool PointLight::is_delta() const {
    return true;
  }
}

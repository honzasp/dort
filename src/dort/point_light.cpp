#include "dort/point_light.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum PointLight::sample_radiance(const Point& pt, float pt_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow) const
  {
    out_wi = normalize(pt - this->pt);
    out_pdf = 1.f;
    out_shadow.init_point_point(this->pt, pt_epsilon, pt, 0.f);
    return this->intensity / length_squared(pt - this->pt);
  }
}

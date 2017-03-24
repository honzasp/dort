#include "dort/beam_light.hpp"

namespace dort {
  BeamLight::BeamLight(const Point& pt, const Vector& dir,
      const Spectrum& radiance, uint32_t num_samples):
    Light(LightFlags(LIGHT_DELTA_POS | LIGHT_DELTA_DIR), num_samples),
    pt(pt), dir(normalize(dir)), radiance(radiance)
  { }

  Spectrum BeamLight::sample_ray_radiance(const Scene&, 
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample) const
  {
    out_ray = Ray(this->pt, this->dir, 0.f);
    out_nn = Normal(this->dir);
    out_pos_pdf = 1.f;
    out_dir_pdf = 1.f;
    return this->radiance;
  }

  Spectrum BeamLight::sample_pivot_radiance(const Point& pivot, float,
      Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
      float& out_dir_pdf, ShadowTest& out_shadow, LightSample) const
  {
    out_wi = normalize(this->pt - pivot);
    out_p = this->pt;
    out_nn = -Normal(out_wi);
    out_p_epsilon = 0.f;
    out_dir_pdf = 0.f;
    out_shadow.init_invisible();
    return Spectrum(0.f);
  }

  bool BeamLight::sample_point(Point& out_p, float& out_p_epsilon,
      Normal& out_nn, float& out_pos_pdf, LightSample) const
  {
    out_p = this->pt;
    out_p_epsilon = 0.f;
    out_nn = Normal();
    out_pos_pdf = 1.f;
    return true;
  }

  Spectrum BeamLight::eval_radiance(const Point&, const Normal&, const Point&) const {
    return Spectrum(0.f);
  }

  float BeamLight::ray_radiance_pdf(const Scene&, const Point&,
      const Vector&, const Normal&) const 
  {
    return 1.f;
  }

  float BeamLight::pivot_radiance_pdf(const Vector&, const Point&) const {
    return 0.f;
  }

  Spectrum BeamLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum BeamLight::approximate_power(const Scene&) const {
    return this->radiance;
  }
}

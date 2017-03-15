#include "dort/directional_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  DirectionalLight::DirectionalLight(const Vector& dir, const Spectrum& radiance):
    Light(LIGHT_DELTA_DIR, 1),
    direction(normalize(dir)),
    radiance(radiance) 
  {
    coordinate_system(this->direction, this->s, this->t);
  }

  Spectrum DirectionalLight::sample_ray_radiance(const Scene& scene, 
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample sample) const 
  {
    Vec2 disk_p = uniform_disk_sample(sample.uv_pos.x, sample.uv_pos.y);
    Point orig = scene.centroid - Vector(scene.radius * this->direction)
      + disk_p.x * this->s + disk_p.y * this->t;
    out_ray = Ray(orig, this->direction, 0.f);
    out_nn = Normal(this->direction);
    out_pos_pdf = uniform_disk_pdf() / (PI * square(scene.radius));
    out_dir_pdf = 1.f;
    return this->radiance;
  }

  Spectrum DirectionalLight::sample_pivot_radiance(const Point& pivot,
      float pivot_epsilon, Vector& out_wi, float& out_dir_pdf,
      ShadowTest& out_shadow, LightSample) const
  {
    out_wi = -this->direction;
    out_dir_pdf = 1.f;
    out_shadow.init_point_dir(pivot, pivot_epsilon, out_wi);
    return this->radiance;
  }

  bool DirectionalLight::sample_point(Point&, float&,
      Normal&, float&, LightSample) const 
  {
    return false;
  }

  Spectrum DirectionalLight::eval_radiance(const Point&,
      const Normal&, const Point&) const
  {
    return Spectrum(0.f);
  }

  float DirectionalLight::ray_origin_radiance_pdf(const Scene&,
      const Point&, const Vector&) const 
  {
    return 0.f;
  }

  float DirectionalLight::ray_dir_radiance_pdf(const Scene&,
      const Vector&, const Point&, const Normal&) const
  {
    return 0.f;
  }

  float DirectionalLight::pivot_radiance_pdf(const Point&, const Vector&) const {
    return 0.f;
  }

  Spectrum DirectionalLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum DirectionalLight::approximate_power(const Scene& scene) const {
    return PI * square(scene.radius) * this->radiance;
  }
}

#include "dort/infinite_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  Spectrum InfiniteLight::sample_ray_radiance(const Scene& scene, 
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample sample) const
  {
    Vec3 dir = uniform_sphere_sample(sample.uv_dir.x, sample.uv_dir.y);
    Vec2 disk_p = uniform_disk_sample(sample.uv_pos.x, sample.uv_pos.y);
    const Point& center = scene.centroid;
    float radius = scene.radius;

    Vector s, t;
    coordinate_system(Vector(dir), s, t);
    Point orig = center + radius * (-Vector(dir) + disk_p.x * s + disk_p.y * t);

    out_ray = Ray(orig, Vector(dir), 0.f);
    out_nn = Normal(dir);
    out_pos_pdf = INV_PI / square(radius);
    out_dir_pdf = INV_FOUR_PI;
    return this->radiance;
  }

  Spectrum InfiniteLight::sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
      Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
      float& out_dir_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    out_wi = Vector(uniform_sphere_sample(sample.uv_pos.x, sample.uv_pos.y));
    out_p = Point(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
    out_nn = Normal(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
    out_p_epsilon = SIGNALING_NAN;
    out_dir_pdf = INV_FOUR_PI;
    out_shadow.init_point_dir(pivot, pivot_epsilon, out_wi);
    return this->radiance;
  }

  bool InfiniteLight::sample_point(Point&, float&, Normal&, float&, LightSample) const {
    return false;
  }

  Spectrum InfiniteLight::eval_radiance(const Point&,
      const Normal&, const Point&) const
  {
    return Spectrum(0.f);
  }

  float InfiniteLight::ray_radiance_pdf(const Scene& scene, const Point&,
      const Vector&, const Normal&) const
  {
    return INV_PI * INV_FOUR_PI / square(scene.radius);
  }

  float InfiniteLight::pivot_radiance_pdf(const Vector&, const Point&) const {
    return INV_FOUR_PI;
  }

  Spectrum InfiniteLight::background_radiance(const Ray&) const {
    return this->radiance;
  }

  Spectrum InfiniteLight::approximate_power(const Scene& scene) const {
    return PI * square(scene.radius) * this->radiance;
  }
}

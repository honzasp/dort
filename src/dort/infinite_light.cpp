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
    Point orig = center - Vector(radius * dir) + disk_p.x * s + disk_p.y * t;

    out_ray = Ray(orig, Vector(dir), 0.f);
    out_nn = Normal(dir);
    out_pos_pdf = uniform_sphere_pdf();
    out_dir_pdf = uniform_disk_pdf();
    return this->radiance;
  }

  Spectrum InfiniteLight::sample_pivot_radiance(const Point& pivot,
      float pivot_epsilon, Vector& out_wi, float& out_dir_pdf,
      ShadowTest& out_shadow, LightSample sample) const
  {
    out_wi = Vector(uniform_sphere_sample(sample.uv_pos.x, sample.uv_pos.y));
    out_dir_pdf = uniform_sphere_pdf();
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

  float InfiniteLight::ray_origin_radiance_pdf(const Scene&,
      const Point&, const Vector&) const 
  {
    return 0.f;
  }

  float InfiniteLight::ray_dir_radiance_pdf(const Scene&,
      const Vector&, const Point&, const Normal&) const
  {
    return 0.f;
  }

  float InfiniteLight::pivot_radiance_pdf(const Point&, const Vector&) const {
    return uniform_sphere_pdf();
  }

  Spectrum InfiniteLight::background_radiance(const Ray&) const {
    return this->radiance;
  }

  Spectrum InfiniteLight::approximate_power(const Scene& scene) const {
    return PI * square(scene.radius) * this->radiance;
  }
}

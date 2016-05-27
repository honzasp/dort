#include "dort/infinite_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  Spectrum InfiniteLight::sample_ray_radiance(const Scene& scene, 
      Ray& out_ray, Normal& out_nn, float& out_pdf,
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
    out_pdf = uniform_sphere_pdf() * uniform_disk_pdf();
    return this->radiance;
  }

  Spectrum InfiniteLight::sample_radiance(const Point& eye, float eye_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    out_wi = Vector(uniform_sphere_sample(sample.uv_pos.x, sample.uv_pos.y));
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

  Spectrum InfiniteLight::approximate_power(const Scene& scene) const {
    return PI * square(scene.radius) * this->radiance;
  }
}

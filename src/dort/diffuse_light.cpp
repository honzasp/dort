#include "dort/diffuse_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  DiffuseLight::DiffuseLight(std::shared_ptr<Shape> shape,
      const Transform& shape_to_world,
      Spectrum radiance,
      uint32_t num_samples):
    AreaLight(LIGHT_AREA, num_samples),
    shape(std::move(shape)),
    shape_to_world(shape_to_world),
    radiance(radiance)
  {
    this->area = this->shape->area();
  }

  Spectrum DiffuseLight::sample_ray_radiance(const Scene&, 
      Ray& out_ray, Normal& out_nn, float& out_pdf,
      LightRaySample sample) const
  {
    Normal shape_n;
    float ray_epsilon;
    Point shape_p = this->shape->sample_point(sample.uv_pos.x, sample.uv_pos.y,
        shape_n, ray_epsilon);
    Vector shape_wo = uniform_hemisphere_sample(sample.uv_dir.x, sample.uv_dir.y);
    if(dot(shape_wo, shape_n) < 0.f) {
      shape_wo = -shape_wo;
    }

    Normal world_n = this->shape_to_world.apply(shape_n);
    Point world_p = this->shape_to_world.apply(shape_p);
    Vector world_wo = this->shape_to_world.apply(shape_wo);

    out_ray = Ray(world_p, world_wo, ray_epsilon);
    out_nn = normalize(world_n);
    out_pdf = this->shape->point_pdf(shape_p) * INV_TWO_PI;
    return this->radiance;
  }

  Spectrum DiffuseLight::sample_radiance(const Point& eye, float eye_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    Point shape_eye = this->shape_to_world.apply_inv(eye);
    Normal shape_n;
    Point shape_p = this->shape->sample_point_eye(shape_eye,
        sample.uv_pos.x, sample.uv_pos.y, shape_n);

    Vector shape_wi = normalize(shape_p - shape_eye);
    Point world_p = this->shape_to_world.apply(shape_p);
    out_wi = normalize(world_p - eye);
    out_pdf = this->shape->point_eye_pdf(shape_eye, shape_wi);
    out_shadow.init_point_point(eye, eye_epsilon, world_p, 1e-1f); // TODO: epsilon

    if(dot(shape_wi, shape_n) < 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  float DiffuseLight::radiance_pdf(const Point& eye, const Vector& wi) const {
    Point shape_eye = this->shape_to_world.apply_inv(eye);
    Vector shape_wi = normalize(this->shape_to_world.apply_inv(wi));
    return this->shape->point_eye_pdf(shape_eye, shape_wi);
  }

  Spectrum DiffuseLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum DiffuseLight::emitted_radiance(const Point&,
      const Normal& n, const Vector& wo) const 
  {
    Normal shape_n = this->shape_to_world.apply_inv(n);
    Vector shape_wo = this->shape_to_world.apply_inv(wo);

    if(dot(shape_wo, shape_n) > 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  Spectrum DiffuseLight::approximate_power(const Scene&) const {
    return this->shape->area() * this->radiance;
  }
}


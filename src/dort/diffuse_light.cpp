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
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample sample) const
  {
    Normal shape_n;
    float ray_epsilon;
    float pos_pdf;
    Point shape_p = this->shape->sample_point(sample.uv_pos, pos_pdf,
        shape_n, ray_epsilon);
    Vector shape_wo = Vector(uniform_hemisphere_sample(sample.uv_dir.x, sample.uv_dir.y));
    if(dot(shape_wo, shape_n) < 0.f) {
      shape_wo = -shape_wo;
    }

    Normal world_n = this->shape_to_world.apply(shape_n);
    Point world_p = this->shape_to_world.apply(shape_p);
    Vector world_wo = this->shape_to_world.apply(shape_wo);

    out_ray = Ray(world_p, world_wo, ray_epsilon);
    out_nn = normalize(world_n);
    out_pos_pdf = pos_pdf;
    out_dir_pdf = INV_TWO_PI;
    return this->radiance;
  }

  Spectrum DiffuseLight::sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
      Vector& out_wi, float& out_dir_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    Point shape_pivot = this->shape_to_world.apply_inv(pivot);
    Normal shape_n;
    float epsilon;
    float dir_pdf;
    Point shape_p = this->shape->sample_point_pivot(shape_pivot,
        sample.uv_pos, dir_pdf, shape_n, epsilon);

    Vector shape_wi = normalize(shape_p - shape_pivot);
    Point world_p = this->shape_to_world.apply(shape_p);
    out_wi = normalize(world_p - pivot);
    out_dir_pdf = dir_pdf;
    out_shadow.init_point_point(pivot, pivot_epsilon, world_p, epsilon);

    if(dot(shape_wi, shape_n) < 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  float DiffuseLight::pivot_radiance_pdf(const Point& pivot, const Vector& wi) const {
    Point shape_pivot = this->shape_to_world.apply_inv(pivot);
    Vector shape_wi = normalize(this->shape_to_world.apply_inv(wi));
    return this->shape->point_pivot_pdf(shape_pivot, shape_wi);
  }

  Spectrum DiffuseLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum DiffuseLight::approximate_power(const Scene&) const {
    return this->shape->area() * this->radiance;
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
}


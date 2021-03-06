#include "dort/diffuse_light.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  DiffuseLight::DiffuseLight(std::shared_ptr<Shape> shape,
      const Transform& shape_to_world, Spectrum radiance):
    Light(LIGHT_AREA),
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
      Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
      float& out_dir_pdf, ShadowTest& out_shadow, LightSample sample) const
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
    out_p = world_p;
    out_nn = this->shape_to_world.apply(shape_n);
    out_p_epsilon = epsilon;
    out_dir_pdf = dir_pdf;
    out_shadow.init_point_point(pivot, pivot_epsilon, world_p, epsilon);

    if(dot(shape_wi, shape_n) < 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  bool DiffuseLight::sample_point(Point& out_p, float& p_epsilon,
      Normal& out_nn, float& out_pos_pdf, LightSample sample) const
  {
    float pos_pdf;
    float ray_epsilon;
    Normal shape_n;
    Point shape_p = this->shape->sample_point(sample.uv_pos,
        pos_pdf, shape_n, ray_epsilon);

    out_p = this->shape_to_world.apply(shape_p);
    p_epsilon = ray_epsilon;
    out_nn = normalize(this->shape_to_world.apply(shape_n));
    out_pos_pdf = pos_pdf;
    return true;
  }

  Spectrum DiffuseLight::eval_radiance(const Point& pt,
      const Normal& nn, const Point& pivot) const
  {
    Normal shape_n = this->shape_to_world.apply_inv(nn);
    Vector shape_wo = this->shape_to_world.apply_inv(pivot - pt);
    if(dot(shape_n, shape_wo) >= 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  float DiffuseLight::ray_radiance_pdf(const Scene&, const Point& origin_gen,
      const Vector& dir_gen, const Normal& nn) const
  {
    if(dot(dir_gen, nn) <= 0.f) { return 0.f; }
    Point shape_p = this->shape_to_world.apply_inv(origin_gen);
    float dir_pdf = INV_TWO_PI;
    float p_pdf = this->shape->point_pdf(shape_p);
    return p_pdf * dir_pdf;
  }

  float DiffuseLight::pivot_radiance_pdf(const Vector& wi_gen,
      const Point& pivot_fix) const 
  {
    Point shape_pivot = this->shape_to_world.apply_inv(pivot_fix);
    Vector shape_wi = normalize(this->shape_to_world.apply_inv(wi_gen));
    return this->shape->point_pivot_pdf(shape_pivot, shape_wi);
  }

  Spectrum DiffuseLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  Spectrum DiffuseLight::approximate_power(const Scene&) const {
    return this->shape->area() * this->radiance;
  }
}


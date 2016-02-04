#include "dort/diffuse_light.hpp"

namespace dort {
  DiffuseLight::DiffuseLight(std::shared_ptr<Shape> shape,
      const Transform& shape_to_world,
      Spectrum radiance,
      uint32_t num_samples):
    AreaLight(num_samples),
    shape(std::move(shape)),
    shape_to_world(shape_to_world),
    radiance(radiance)
  {
    this->area = this->shape->area();
  }

  Spectrum DiffuseLight::sample_radiance(const Point& eye, float eye_epsilon,
      Vector& out_wi, float& out_pdf, ShadowTest& out_shadow,
      Rng& rng) const
  {
    Point shape_eye = this->shape_to_world.apply_inv(eye);
    Normal shape_n;
    Point shape_p = this->shape->sample_point_eye(shape_eye,
        rng.uniform_float(), rng.uniform_float(), shape_n);

    Vector shape_wi = normalize(shape_p - shape_eye);
    Point world_p = this->shape_to_world.apply(shape_p);
    out_wi = normalize(world_p - eye);
    out_pdf = this->shape->point_eye_pdf(shape_eye, shape_wi);
    out_shadow.init_point_point(eye, eye_epsilon, world_p, 1e-1f); // TODO: epsilon

    /*
    std::printf("sample from eye %f,%f,%f\n", eye.v.x, eye.v.y, eye.v.z);
    std::printf("  shape_eye %f,%f,%f\n", shape_eye.v.x, shape_eye.v.y, shape_eye.v.z);
    std::printf("  shape_p %f,%f,%f\n", shape_p.v.x, shape_p.v.y, shape_p.v.z);
    std::printf("  world_p %f,%f,%f\n", world_p.v.x, world_p.v.y, world_p.v.z);
    std::printf("  shape_wi %f,%f,%f\n", shape_wi.v.x, shape_wi.v.y, shape_wi.v.z);
    std::printf("  out_wi %f,%f,%f\n", out_wi.v.x, out_wi.v.y, out_wi.v.z);
    std::printf("  out_pdf %f\n", out_pdf);
    std::printf("  dot %f\n", dot(out_wi, shape_n));
    */

    if(dot(out_wi, shape_n) < 0.f) {
      return this->radiance;
    } else {
      return Spectrum(0.f);
    }
  }

  float DiffuseLight::radiance_pdf(const Point& eye, const Vector& wi) const {
    return this->shape->point_eye_pdf(eye, wi);
  }

  Spectrum DiffuseLight::background_radiance(const Ray&) const {
    return Spectrum(0.f);
  }

  bool DiffuseLight::is_delta() const {
    return false;
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


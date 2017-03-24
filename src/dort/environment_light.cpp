#include "dort/environment_light.hpp"
#include "dort/image.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  EnvironmentLight::EnvironmentLight(std::shared_ptr<Image<PixelRgbFloat>> image,
      const Spectrum& scale, const Vector& up, const Vector& fwd, uint32_t num_samples):
    Light(LightFlags(LIGHT_BACKGROUND | LIGHT_DISTANT), num_samples),
    image(image), scale(scale)
  {
    this->up = normalize(up);
    this->t = normalize(cross(up, fwd));
    this->s = cross(this->up, this->t);

    std::vector<float> values(image->x_res * image->y_res);
    Spectrum value_sum(0.f);
    float value_weight = 0.f;
    for(uint32_t y = 0; y < image->y_res; ++y) {
      float theta = (float(y) + 0.5f) / float(image->y_res) * PI;
      float cos_theta = clamp(cos(theta), 0.f, 1.f);
      for(uint32_t x = 0; x < image->x_res; ++x) {
        // TODO: once we use bilinear interpolation from images, the pdfs must
        // be blurred a bit (see PBRT page 848)
        Spectrum value = PixelRgbFloat::to_rgb(image->get_pixel(x, y));
        values.at(y * image->x_res + x) = abs(value.average() * cos_theta);
        value_sum += value * cos_theta;
        value_weight += cos_theta;
      }
    }
    this->distrib = PiecewiseDistrib2d(image->x_res, image->y_res, std::move(values));
    this->average_radiance = this->scale * value_sum / value_weight;
  }

  Spectrum EnvironmentLight::sample_ray_radiance(const Scene& scene, 
      Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
      LightRaySample sample) const
  {
    Vec2 dir_uv;
    float dir_pdf;
    Vector dir = this->sample_dir(sample.uv_dir, dir_uv, dir_pdf);
    Vec2 disk_p = uniform_disk_sample(sample.uv_pos.x, sample.uv_pos.y);

    Vector s, t;
    coordinate_system(dir, s, t);
    Point orig = scene.centroid + scene.radius * 
      (dir + disk_p.x * s + disk_p.y * t);

    out_ray = Ray(orig, -dir, 0.f);
    out_nn = Normal(-dir);
    out_pos_pdf = INV_PI / square(scene.radius);
    out_dir_pdf = dir_pdf;
    return this->get_radiance(dir_uv);
  }

  Spectrum EnvironmentLight::sample_pivot_radiance(const Point& pivot,
      float pivot_epsilon, Vector& out_wi, Point& out_p, Normal& out_nn,
      float& out_p_epsilon, float& out_dir_pdf, ShadowTest& out_shadow,
      LightSample sample) const
  {
    Vec2 dir_uv;
    out_wi = this->sample_dir(sample.uv_pos, dir_uv, out_dir_pdf);
    out_p = Point(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
    out_nn = Normal(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
    out_p_epsilon = SIGNALING_NAN;
    out_shadow.init_point_dir(pivot, pivot_epsilon, out_wi);
    return this->get_radiance(dir_uv);
  }

  bool EnvironmentLight::sample_point(Point&, float&, Normal&, float&, LightSample) const {
    return false;
  }

  Spectrum EnvironmentLight::eval_radiance(const Point&,
      const Normal&, const Point&) const
  {
    return Spectrum(0.f);
  }

  float EnvironmentLight::ray_radiance_pdf(const Scene& scene,
      const Point&, const Vector& dir_gen, const Normal&) const
  {
    return this->dir_pdf(-dir_gen) * INV_PI / square(scene.radius);
  }

  float EnvironmentLight::pivot_radiance_pdf(const Vector& wi_gen, const Point&) const {
    return this->dir_pdf(wi_gen);
  }

  Spectrum EnvironmentLight::background_radiance(const Ray& ray) const {
    float sin_theta;
    Vec2 dir_uv = this->dir_to_dir_uv(ray.dir, sin_theta);
    return this->get_radiance(dir_uv);
  }

  Spectrum EnvironmentLight::approximate_power(const Scene& scene) const {
    return PI * square(scene.radius) * this->average_radiance;
  }

  Vector EnvironmentLight::sample_dir(Vec2 uv,
      Vec2& out_dir_uv, float& out_dir_pdf) const 
  {
    Vec2 dir_xy = this->distrib.sample(uv);
    Vec2 dir_uv(dir_xy.x / float(this->image->x_res),
        dir_xy.y / float(this->image->y_res));
    float sin_theta;
    Vector dir = this->dir_uv_to_dir(dir_uv, sin_theta);
    out_dir_uv = dir_uv;
    out_dir_pdf = sin_theta == 0.f ? 0.f
      : this->dir_uv_pdf(dir_uv) / (2.f * square(PI) * sin_theta);
    return dir;
  }

  float EnvironmentLight::dir_pdf(const Vector& dir) const {
    float sin_theta;
    Vec2 dir_uv = this->dir_to_dir_uv(dir, sin_theta);
    if(sin_theta == 0.f) { return 0.f; }
    return this->dir_uv_pdf(dir_uv) / (2.f * square(PI) * sin_theta);
  }

  float EnvironmentLight::dir_uv_pdf(Vec2 dir_uv) const {
    float x = dir_uv.x * float(this->image->x_res);
    float y = dir_uv.y * float(this->image->y_res);
    float xy_pdf = this->distrib.pdf(Vec2(x, y));
    assert(xy_pdf >= 0.f);
    return xy_pdf * float(this->distrib.area());
  }

  Vector EnvironmentLight::dir_uv_to_dir(Vec2 dir_uv, float& out_sin_theta) const {
    float theta = dir_uv.y * PI;
    float phi = (dir_uv.x - 0.5f) * TWO_PI;
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    float cos_phi = cos(phi);
    float sin_phi = sin(phi);

    out_sin_theta = clamp(sin_theta, 0.f, 1.f);
    return this->up * cos_theta 
      + this->s * (sin_theta * cos_phi)
      + this->t * (sin_theta * sin_phi);
  }

  Vec2 EnvironmentLight::dir_to_dir_uv(const Vector& dir, float& out_sin_theta) const {
    float up = dot(dir, this->up);
    float s = dot(dir, this->s);
    float t = dot(dir, this->t);

    float phi = atan2(t, s);
    float theta = acos(clamp(up, -1.f, 1.f));
    out_sin_theta = sqrt(max(0.f, 1.f - square(up)));
    return Vec2(phi * INV_TWO_PI + 0.5f, theta * INV_PI);
  }

  Spectrum EnvironmentLight::get_radiance(Vec2 dir_uv) const {
    uint32_t x = floor_int32(dir_uv.x * float(this->image->x_res));
    uint32_t y = floor_int32(dir_uv.y * float(this->image->y_res));
    if(x >= this->image->x_res || y >= this->image->y_res) { return Spectrum(0.f); }
    return this->scale * PixelRgbFloat::to_rgb(this->image->get_pixel(x, y));
  }
}

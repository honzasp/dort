#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/light.hpp"

namespace dort {
  CameraSample::CameraSample(Rng& rng) {
    this->uv_lens = Vec2(rng.uniform_float(), rng.uniform_float());
  }

  CameraSample::CameraSample(Sampler& sampler, const CameraSamplesIdxs& idxs, uint32_t n) {
    this->uv_lens = sampler.get_array_2d(idxs.uv_lens_idx).at(n);
  }

  CameraSamplesIdxs CameraSample::request(Sampler& sampler, uint32_t count) {
    CameraSamplesIdxs idxs;
    idxs.uv_lens_idx = sampler.request_array_2d(count);
    idxs.count = count;
    return idxs;
  }


  Vec2 Camera::film_to_normal(Vec2 film_res, Vec2 film_pos) {
    Vec2 center_pos = film_pos / film_res - Vec2(0.5f, 0.5f);
    if(film_res.x > film_res.y) {
      return Vec2(center_pos.x, center_pos.y * film_res.y / film_res.x);
    } else {
      return Vec2(center_pos.x * film_res.x / film_res.y, center_pos.y);
    }
  }

  Vec2 Camera::normal_to_film(Vec2 film_res, Vec2 normal_pos) {
    Vec2 center_pos;
    if(film_res.x > film_res.y) {
      center_pos = Vec2(normal_pos.x, normal_pos.y * film_res.x / film_res.y);
    } else {
      center_pos = Vec2(normal_pos.x * film_res.y / film_res.x, normal_pos.y);
    }
    return (center_pos + Vec2(0.5f, 0.5f)) * film_res;
  }


  OrthographicCamera::OrthographicCamera(const Transform& camera_to_world,
      float dimension):
    Camera(CameraFlags(CAMERA_LENS_POS_DELTA | CAMERA_LENS_DIR_DELTA 
        | CAMERA_FILM_PIVOT_POS_DELTA), camera_to_world),
    dimension(dimension)
  { }

  Spectrum OrthographicCamera::sample_ray_importance(Vec2 film_res, Vec2 film_pos,
      Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
      CameraSample) const
  {
    Vec2 lens_pos = Camera::film_to_normal(film_res, film_pos) * this->dimension;
    Point world_pos = this->camera_to_world.apply(
        Point(lens_pos.x, lens_pos.y, 0.f));
    Vector world_dir = normalize(this->camera_to_world.apply(
        Vector(0.f, 0.f, 1.f)));
    out_ray = Ray(world_pos, world_dir);
    out_pos_pdf = 1.f;
    out_dir_pdf = 1.f;
    return Spectrum(1.f);
  }

  Spectrum OrthographicCamera::sample_pivot_importance(Vec2 film_res,
      const Point& pivot, float pivot_epsilon, Vector& out_wo, float& out_dir_pdf,
      ShadowTest& out_shadow, Vec2& out_film_pos, CameraSample) const
  {
    Point lens_pos = this->camera_to_world.apply_inv(pivot);
    if(lens_pos.v.z < 0.f) {
      out_wo = Vector();
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
    }
    Vec2 normal_pos = Vec2(lens_pos.v.x, lens_pos.v.y) / this->dimension;
    Vec2 film_pos = Camera::normal_to_film(film_res, normal_pos);

    out_wo = this->camera_to_world.apply(Vector(0.f, 0.f, 1.f));
    out_dir_pdf = 1.f;
    out_shadow.init_point_point(pivot, pivot_epsilon,
        this->camera_to_world.apply(Point(lens_pos.v.x, lens_pos.v.y, 0.f)), 0.f);
    out_film_pos = film_pos;
    return Spectrum(1.f);
  }

  float OrthographicCamera::ray_dir_importance_pdf(Vec2,
      const Vector&, const Point&) const
  {
    return 0.f;
  }


  PinholeCamera::PinholeCamera(const Transform& camera_to_world, float fov):
    Camera(CameraFlags(CAMERA_LENS_POS_DELTA | CAMERA_LENS_DIR_DELTA
        | CAMERA_FILM_PIVOT_POS_DELTA), camera_to_world)
  {
    this->world_origin = this->camera_to_world.apply(Point(0.f, 0.f, 0.f));
    this->project_dimension = 2.f * tan(0.5f * fov);
  }

  Spectrum PinholeCamera::sample_ray_importance(Vec2 film_res, Vec2 film_pos,
      Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
      CameraSample) const
  {
    Vec2 normal_pos = Camera::film_to_normal(film_res, film_pos);
    Vec2 project_pos = normal_pos * this->project_dimension;
    Vector world_dir = normalize(this->camera_to_world.apply(
        Vector(project_pos.x, project_pos.y, 1.f)));
    out_ray = Ray(this->world_origin, world_dir);
    out_pos_pdf = 1.f;
    out_dir_pdf = 1.f;
    return Spectrum(1.f);
  }

  Spectrum PinholeCamera::sample_pivot_importance(Vec2 film_res,
      const Point& pivot, float pivot_epsilon,
      Vector& out_wo, float& out_dir_pdf, ShadowTest& out_shadow,
      Vec2& out_film_pos, CameraSample) const
  {
    Point camera_pivot = this->camera_to_world.apply_inv(pivot);
    if(camera_pivot.v.z <= 0.f) {
      out_wo = Vector();
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
    }
    Vec2 project_pos = Vec2(camera_pivot.v.x, camera_pivot.v.y) / camera_pivot.v.z;
    Vec2 normal_pos = project_pos / this->project_dimension;
    if(abs(normal_pos.x) > 0.5f || abs(normal_pos.y) > 0.5f) {
      out_wo = Vector();
      out_dir_pdf = 0.f;
      return Spectrum(0.f);
    }
    Vec2 film_pos = Camera::normal_to_film(film_res, normal_pos);

    Vector camera_dir = Vector(camera_pivot.v);
    float cos_theta = camera_dir.v.z / length(camera_dir);
    float distance = length(pivot - this->world_origin);
    out_wo = -normalize(this->camera_to_world.apply(camera_dir));
    out_dir_pdf = square(distance) / cube(cos_theta);
    out_shadow.init_point_point(pivot, pivot_epsilon, this->world_origin, 0.f);
    out_film_pos = film_pos;
    return Spectrum(1.f / square(square(cos_theta)));
  }

  float PinholeCamera::ray_dir_importance_pdf(Vec2,
      const Vector& wi_gen, const Point&) const
  {
    Vector wi_camera = this->camera_to_world.apply_inv(wi_gen);
    if(wi_camera.v.z <= 0.f) { return 0.f; }
    float inv_z = 1.f / wi_camera.v.z;
    Vec2 project_pos(wi_camera.v.x * inv_z, wi_camera.v.y * inv_z);
    Vec2 normal_pos = project_pos / this->project_dimension;

    if(abs(normal_pos.x) <= 0.5 && abs(normal_pos.y) <= 0.5) {
      float cos_theta = wi_camera.v.z / length(wi_camera);
      return 1.f / cube(cos_theta);
    } else {
      return 0.f;
    }
  }
}

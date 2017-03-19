#include "dort/light.hpp"
#include "dort/pinhole_camera.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  PinholeCamera::PinholeCamera(const Transform& camera_to_world, float fov):
    Camera(CameraFlags(CAMERA_POS_DELTA), camera_to_world)
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
    Vector camera_dir = Vector(project_pos.x, project_pos.y, 1.f);
    Vector world_dir = normalize(this->camera_to_world.apply(camera_dir));

    float cos_theta = camera_dir.v.z / length(camera_dir);
    float area = this->get_image_plane_area(film_res);
    out_ray = Ray(this->world_origin, world_dir, 0.f);
    out_pos_pdf = 1.f;
    out_dir_pdf = 1 / (area * cube(cos_theta));
    return Spectrum(1.f / (area * cube(cos_theta)));
  }

  Spectrum PinholeCamera::sample_pivot_importance(Vec2 film_res,
      const Point& pivot, float pivot_epsilon,
      Point& out_p, Vec2& out_film_pos, float& out_p_pdf, 
      ShadowTest& out_shadow, CameraSample) const
  {
    out_p = this->world_origin;
    out_p_pdf = 1.f;

    Vec3 camera_pivot = this->camera_to_world.apply_inv(pivot).v;
    if(camera_pivot.z <= 0.f) {
      return Spectrum(0.f);
    }

    Vec2 film_pos;
    if(!this->get_film_pos(film_res, camera_pivot, film_pos)) {
      return Spectrum(0.f);
    }

    float area = this->get_image_plane_area(film_res);
    out_shadow.init_point_point(pivot, pivot_epsilon, this->world_origin, 0.f);
    out_film_pos = film_pos;
    return Spectrum(1.f / area);
  }

  Point PinholeCamera::sample_point(float& out_pos_pdf, CameraSample) const {
    out_pos_pdf = 1.f;
    return this->world_origin;
  }

  Spectrum PinholeCamera::eval_importance(Vec2 film_res, const Point&,
      const Vector& wi, Vec2& out_film_pos) const
  {
    Vec3 camera_pivot = this->camera_to_world.apply_inv(wi).v;
    if(camera_pivot.z <= 0.f) {
      out_film_pos = Vec2();
      return Spectrum(0.f);
    }

    Vec2 film_pos;
    if(!this->get_film_pos(film_res, camera_pivot, film_pos)) {
      out_film_pos = Vec2();
      return Spectrum(0.f);
    }

    float cos_theta = camera_pivot.z / length(camera_pivot);
    float area = this->get_image_plane_area(film_res);
    out_film_pos = film_pos;
    return Spectrum(1.f / (area * cube(cos_theta)));
  }

  float PinholeCamera::ray_importance_pdf(Vec2 film_res,
      const Point&, const Vector& wi_gen) const 
  {
    Vec3 camera_pivot = this->camera_to_world.apply_inv(wi_gen).v;
    if(camera_pivot.z <= 0.f) {
      return 0.f; 
    }

    Vec2 dummy_film_pos;
    if(!this->get_film_pos(film_res, camera_pivot, dummy_film_pos)) {
      return 0.f;
    }

    float cos_theta = camera_pivot.z / length(camera_pivot);
    float area = this->get_image_plane_area(film_res);
    return 1.f / (area * cube(cos_theta));
  }

  float PinholeCamera::pivot_importance_pdf(Vec2, const Point&, const Point&) const {
    return 1.f;
  }

  bool PinholeCamera::get_film_pos(Vec2 film_res,
      const Vec3& pivot, Vec2& out_film_pos) const 
  {
    assert(pivot.z >= 0.f);
    Vec2 project_pos = Vec2(pivot.x, pivot.y) / pivot.z;
    Vec2 normal_pos = project_pos / this->project_dimension;
    if(abs(normal_pos.x) > 0.5f || abs(normal_pos.y) > 0.5f) {
      return false;
    }
    out_film_pos = Camera::normal_to_film(film_res, normal_pos);
    return true;
  }

  float PinholeCamera::get_image_plane_area(Vec2 film_res) const {
    return min(film_res.x, film_res.y) / max(film_res.x, film_res.y) 
      * square(this->project_dimension);
  }
}

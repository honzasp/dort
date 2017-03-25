#include "dort/light.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/spectrum.hpp"
#include "dort/thin_lens_camera.hpp"

namespace dort {
  ThinLensCamera::ThinLensCamera(const Transform& camera_to_world, float fov,
      float lens_radius, float focal_distance):
    Camera(CameraFlags(), camera_to_world),
    lens_radius(lens_radius), focal_distance(focal_distance)
  {
    this->world_origin = this->camera_to_world.apply(Point(0.f, 0.f, 0.f));
    this->project_dimension = 2.f * tan(0.5f * fov);
  }

  Spectrum ThinLensCamera::sample_ray_importance(Vec2 film_res, Vec2 film_pos,
      Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf, CameraSample sample) const
  {
    Vec3 focus_point = this->get_focus_point(film_res, film_pos);
    Vec3 lens_point = this->sample_lens_point(sample.uv_lens);
    float area = this->get_image_plane_area(film_res);
    float cos_theta = focus_point.z / length(focus_point);

    Point world_focus_point = this->camera_to_world.apply(Point(focus_point));
    Point world_lens_point = this->camera_to_world.apply(Point(lens_point));
    float pos_pdf = INV_PI / square(this->lens_radius);
    float dir_pdf = 1.f / (area * cube(cos_theta));
    out_ray = Ray(world_lens_point, world_focus_point - world_lens_point, 0.f);
    out_pos_pdf = pos_pdf;
    out_dir_pdf = dir_pdf;
    return Spectrum(pos_pdf * dir_pdf);
  }

  Spectrum ThinLensCamera::sample_pivot_importance(Vec2 film_res,
      const Point& pivot, float pivot_epsilon,
      Point& out_p, Vec2& out_film_pos, float& out_p_pdf,
      ShadowTest& out_shadow, CameraSample sample) const
  {
    Vec3 lens_point = this->sample_lens_point(sample.uv_lens);
    Vec3 camera_pivot = this->camera_to_world.apply_inv(pivot).v;
    Vec2 film_pos;
    if(!this->get_film_pos(film_res, lens_point, camera_pivot, film_pos)) {
      return Spectrum(0.f);
    }

    float area = this->get_image_plane_area(film_res);
    Point world_p = this->camera_to_world.apply(Point(lens_point));
    out_p = world_p;
    out_film_pos = film_pos;
    out_p_pdf = INV_PI / square(this->lens_radius);
    out_shadow.init_point_point(pivot, pivot_epsilon, world_p, 0.f);
    return Spectrum(INV_PI / (square(this->lens_radius) * area));
  }

  Point ThinLensCamera::sample_point(Vec2,
      float& out_pos_pdf, CameraSample sample) const
  {
    Vec3 lens_point = this->sample_lens_point(sample.uv_lens);
    out_pos_pdf = INV_PI / square(this->lens_radius);
    return this->camera_to_world.apply(Point(lens_point));
  }

  Spectrum ThinLensCamera::eval_importance(Vec2 film_res,
      const Point& p, const Vector& wi, Vec2& out_film_pos) const
  {
    Vec3 lens_point = this->camera_to_world.apply_inv(p).v;
    Vec3 dir = this->camera_to_world.apply_inv(wi).v;
    Vec3 camera_pivot = dir + lens_point;
    Vec2 film_pos;
    if(!this->get_film_pos(film_res, lens_point, camera_pivot, film_pos)) {
      return Spectrum(0.f);
    }

    float cos_theta = dir.z / length(dir);
    float area = this->get_image_plane_area(film_res);
    out_film_pos = film_pos;
    return Spectrum(INV_PI / (square(this->lens_radius) * area * cube(cos_theta)));
  }

  float ThinLensCamera::ray_importance_pdf(Vec2 film_res,
      const Point&, const Vector& wi_gen) const
  {
    Vec3 camera_wi = this->camera_to_world.apply_inv(wi_gen).v;
    float cos_theta = camera_wi.z / length(camera_wi);
    float area = this->get_image_plane_area(film_res);
    float inv_pos_pdf = PI * square(this->lens_radius);
    float inv_dir_pdf = area * cube(cos_theta);
    return 1.f / (inv_pos_pdf * inv_dir_pdf);
  }

  float ThinLensCamera::pivot_importance_pdf(Vec2, const Point&, const Point&) const {
    return INV_PI / square(this->lens_radius);
  }

  Vec3 ThinLensCamera::get_focus_point(Vec2 film_res, Vec2 film_pos) const {
    float film_dimension = max(film_res.x, film_res.y);
    Vec2 plane_pos(
        (film_pos.x - 0.5f * film_res.x) * (this->project_dimension / film_dimension),
        (film_pos.y - 0.5f * film_res.y) * (this->project_dimension / film_dimension));
    return Vec3(plane_pos.x * this->focal_distance,
        plane_pos.y * this->focal_distance, this->focal_distance);
  }

  Vec3 ThinLensCamera::sample_lens_point(Vec2 uv) const {
    Vec2 disk_p = uniform_disk_sample(uv.x, uv.y);
    return Vec3(disk_p.x * this->lens_radius, disk_p.y * this->lens_radius, 0.f);
  }

  float ThinLensCamera::get_image_plane_area(Vec2 film_res) const {
    float aspect = min(film_res.x, film_res.y) / max(film_res.x, film_res.y);
    return square(this->project_dimension) * aspect;
  }

  bool ThinLensCamera::get_film_pos(Vec2 film_res, Vec3 lens_point,
      Vec3 camera_pivot, Vec2& out_film_pos) const
  {
    if(camera_pivot.z <= 0.f) { return false; }
    Vec3 focal_plane_pos = lerp(this->focal_distance / camera_pivot.z,
        lens_point, camera_pivot);
    Vec2 project_pos(focal_plane_pos.x / focal_plane_pos.z,
        focal_plane_pos.y / focal_plane_pos.z);
    Vec2 normal_pos = project_pos / this->project_dimension;
    if(abs(normal_pos.x) > 0.5f || abs(normal_pos.y) > 0.5f) {
      return false;
    }

    Vec2 center_pos = film_res.x > film_res.y
      ? Vec2(normal_pos.x, normal_pos.y * film_res.x / film_res.y)
      : Vec2(normal_pos.x * film_res.y / film_res.x, normal_pos.y);
    out_film_pos = (center_pos + Vec2(0.5f, 0.5f)) * film_res;
    return true;
  }
}

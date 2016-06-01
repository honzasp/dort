#include "dort/camera.hpp"
#include "dort/film.hpp"

namespace dort {
  Transform Camera::screen_to_ndc(Vec2 screen_diagonal) {
    return scale(2.f / screen_diagonal.x, 2.f / screen_diagonal.y, 1.f);
  }

  Ray Camera::cast_ray(Vec2 film_res, Vec2 film_pos) const {
    float res = max(film_res.x, film_res.y);
    Vec2 ndc_scale = 2.f / Vec2(res, -res);
    Vec2 ndc_shift = Vec2(-film_res.x/ res, film_res.y / res);
    Vec2 ndc_pos = film_pos * ndc_scale + ndc_shift;
    return this->generate_ray(ndc_pos);
  }

  OrthographicCamera::OrthographicCamera(
      const Transform& camera_to_world, Vec2 screen_diagonal)
  {
    Transform camera_to_screen = identity();
    this->world_to_ndc = Camera::screen_to_ndc(screen_diagonal) *
      camera_to_screen * camera_to_world.inverse();
  }

  Ray OrthographicCamera::generate_ray(Vec2 ndc) const {
    Point ndc_p(ndc.x, ndc.y, 0.f);
    Vector ndc_dir(0.f, 0.f, 1.f);
    Ray ray(this->world_to_ndc.apply_inv(ndc_p),
        normalize(this->world_to_ndc.apply_inv(ndc_dir)),
        0.f, INFINITY);
    return ray;
  }

  PerspectiveCamera::PerspectiveCamera(
      const Transform& camera_to_world, Vec2 screen_diagonal,
      float fov, float z_near, float z_far)
  {
    Transform camera_to_screen = perspective(fov, z_near, z_far);
    this->world_to_ndc = Camera::screen_to_ndc(screen_diagonal) *
      camera_to_screen * camera_to_world.inverse();
    this->world_orig = camera_to_world.apply(Point(0.f, 0.f, 0.f));
  }

  Ray PerspectiveCamera::generate_ray(Vec2 ndc) const {
    Point ndc_p(ndc.x, ndc.y, 0.f);
    Point world_p(this->world_to_ndc.apply_inv(ndc_p));
    Ray ray(this->world_orig, normalize(world_p - this->world_orig), 0.f, INFINITY);
    return ray;
  }
}

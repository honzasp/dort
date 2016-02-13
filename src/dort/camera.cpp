#include "dort/camera.hpp"
#include "dort/film.hpp"

namespace dort {
  Transform Camera::screen_to_ndc(Vec2 screen_diagonal) {
    return scale(2.f / screen_diagonal.x, 2.f / screen_diagonal.y, 1.f);
  }

  OrthographicCamera::OrthographicCamera(
      const Transform& world_to_camera, Vec2 screen_diagonal)
  {
    Transform camera_to_screen = identity();
    this->world_to_ndc = Camera::screen_to_ndc(screen_diagonal) *
      camera_to_screen * world_to_camera;
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
      const Transform& world_to_camera, Vec2 screen_diagonal,
      float fov, float z_near, float z_far)
  {
    Transform camera_to_screen = perspective(fov, z_near, z_far);
    this->world_to_ndc = Camera::screen_to_ndc(screen_diagonal) *
      camera_to_screen * world_to_camera;
    this->world_orig = world_to_camera.apply_inv(Point(0.f, 0.f, 0.f));
  }

  Ray PerspectiveCamera::generate_ray(Vec2 ndc) const {
    Point ndc_p(ndc.x, ndc.y, 0.f);
    Point world_p(this->world_to_ndc.apply_inv(ndc_p));
    Ray ray(this->world_orig, normalize(world_p - this->world_orig), 0.f, INFINITY);
    return ray;
  }
}

#include "dort/light.hpp"
#include "dort/ortho_camera.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  OrthoCamera::OrthoCamera(const Transform& camera_to_world, float dimension):
    Camera(CameraFlags(CAMERA_DIR_DELTA | CAMERA_DIR_BY_POS_DELTA), camera_to_world),
    dimension(dimension)
  { 
    this->world_dir = this->camera_to_world.apply(Vector(0.f, 0.f, 1.f));
  }

  Spectrum OrthoCamera::sample_ray_importance(Vec2 film_res, Vec2 film_pos,
      Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
      CameraSample) const
  {
    Vec2 plane_pos = this->get_plane_pos(film_res, film_pos);
    Point world_origin = this->camera_to_world.apply(
        Point(plane_pos.x, plane_pos.y, 0.f));
    float area = this->get_image_plane_area(film_res);

    out_ray = Ray(world_origin, this->world_dir, 0.f);
    out_pos_pdf = 1.f / area;
    out_dir_pdf = 1.f;
    return Spectrum(1.f / area);
  }

  Spectrum OrthoCamera::sample_pivot_importance(Vec2 film_res,
      const Point& pivot, float pivot_epsilon,
      Point& out_p, Vec2& out_film_pos, float& out_p_pdf,
      ShadowTest& out_shadow, CameraSample) const
  {
    Vec3 camera_pivot = this->camera_to_world.apply_inv(pivot).v;
    Vec2 film_pos;
    if(!this->get_film_pos(film_res, camera_pivot, film_pos)) {
      return Spectrum(0.f);
    }
    Vec2 plane_pos = this->get_plane_pos(film_res, film_pos);
    Point world_p = this->camera_to_world.apply(
        Point(plane_pos.x, plane_pos.y, 0.f));
    float area = this->get_image_plane_area(film_res);

    out_p = world_p;
    out_film_pos = film_pos;
    out_p_pdf = 1.f / length_squared(world_p - pivot);
    out_shadow.init_point_point(pivot, pivot_epsilon, world_p, 0.f);
    return Spectrum(1.f / area);
  }

  Point OrthoCamera::sample_point(Vec2 film_res,
      float& out_pos_pdf, CameraSample sample) const
  {
    Vec2 film_pos = sample.uv_lens * film_res;
    Vec2 plane_pos = this->get_plane_pos(film_res, film_pos);
    out_pos_pdf = 1.f / this->get_image_plane_area(film_res);
    return this->camera_to_world.apply(Point(plane_pos.x, plane_pos.y, 0.f));
  }

  Spectrum OrthoCamera::eval_importance(Vec2, const Point&, const Vector&, Vec2&) const {
    return Spectrum(0.f);
  }

  float OrthoCamera::ray_importance_pdf(Vec2 film_res, const Point&,const Vector&) const {
    return 1.f / this->get_image_plane_area(film_res);
  }

  float OrthoCamera::pivot_importance_pdf(Vec2,
      const Point& p_gen, const Point& pivot_fix) const 
  {
    return 1.f / length_squared(p_gen - pivot_fix);
  }

  bool OrthoCamera::get_film_pos(Vec2 film_res, const Vec3& pivot,
      Vec2& out_film_pos) const
  {
    if(pivot.z < 0.f) { return false; }
    float film_dimension = max(film_res.x, film_res.y);
    out_film_pos.x = (pivot.x / this->dimension + 0.5f) * film_dimension;
    out_film_pos.y = (pivot.y / this->dimension + 0.5f) * film_dimension;
    return out_film_pos.x >= 0.f && out_film_pos.x <= film_res.x
        && out_film_pos.y >= 0.f && out_film_pos.y <= film_res.y;
  }

  Vec2 OrthoCamera::get_plane_pos(Vec2 film_res, Vec2 film_pos) const {
    float film_dimension = max(film_res.x, film_res.y);
    return Vec2((film_pos.x - 0.5f * film_res.x) * (this->dimension / film_dimension),
                (film_pos.y - 0.5f * film_res.y) * (this->dimension / film_dimension));
  }

  float OrthoCamera::get_image_plane_area(Vec2 film_res) const {
    float aspect = min(film_res.x, film_res.y) / max(film_res.x, film_res.y);
    return square(this->dimension) * aspect;
  }
}

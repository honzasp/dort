#pragma once
#include "dort/rect.hpp"
#include "dort/transform.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  class Camera {
  public:
    virtual Ray generate_ray(Vec2 ndc) const = 0;
    static Transform screen_to_ndc(Vec2 screen_diagonal);
    Ray cast_ray(Vec2 film_res, Vec2 film_pos) const;
  };

  class OrthographicCamera final: public Camera {
    Transform world_to_ndc;
  public:
    OrthographicCamera(const Transform& camera_to_world, Vec2 screen_diagonal);
    virtual Ray generate_ray(Vec2 ndc) const override final;
  };

  class PerspectiveCamera final: public Camera {
    Transform world_to_ndc;
    Point world_orig;
  public:
    PerspectiveCamera(const Transform& camera_to_world, Vec2 screen_diagonal,
        float fov, float z_near, float z_far);
    virtual Ray generate_ray(Vec2 ndc) const override final;
  };
}

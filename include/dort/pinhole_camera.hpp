#pragma once
#include "dort/camera.hpp"

namespace dort {
  /// Pinhole camera has a pinhole lens at the origin of the camera space and
  /// projects to the plane z = 1. The angle visible in the longer film
  /// dimension is given as the fov parameter;
  class PinholeCamera final: public Camera {
    Point world_origin;
    float project_dimension;
  public:
    PinholeCamera(const Transform& camera_to_world, float fov);

    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const override final;
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Point& out_p, Vec2& out_film_pos, float& out_p_pdf,
        ShadowTest& out_shadow, CameraSample sample) const override final;
    virtual Point sample_point(Vec2 film_res,
        float& out_pos_pdf, CameraSample sample) const override final;
    virtual Spectrum eval_importance(Vec2 film_res,
        const Point& p, const Vector& wi, Vec2& out_film_pos) const override final;

    virtual float ray_importance_pdf(Vec2 film_res,
        const Point& origin_gen, const Vector& wi_gen) const override final;
    virtual float pivot_importance_pdf(Vec2 film_res,
        const Point& p_gen, const Point& pivot_fix) const override final;
  private:
    bool get_film_pos(Vec2 film_res, const Vec3& pivot,
        Vec2& out_film_pos) const;
    float get_image_plane_area(Vec2 film_res) const;
  };
}

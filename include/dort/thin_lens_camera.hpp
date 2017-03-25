#pragma once
#include "dort/camera.hpp"

namespace dort {
  class ThinLensCamera final: public Camera {
    Point world_origin;
    float project_dimension;
    float lens_radius;
    float focal_distance;
  public:
    ThinLensCamera(const Transform& camera_to_world, float fov,
        float lens_radius, float focal_distance);

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
    Vec3 get_focus_point(Vec2 film_res, Vec2 film_pos) const;
    Vec3 sample_lens_point(Vec2 uv) const;
    float get_image_plane_area(Vec2 film_res) const;
    bool get_film_pos(Vec2 film_res, Vec3 lens_point,
        Vec3 camera_pivot, Vec2& out_film_pos) const;
  };
}

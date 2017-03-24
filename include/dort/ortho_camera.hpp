#pragma once
#include "dort/camera.hpp"

namespace dort {
  /// Orthographic camera maps each film point to a point on the lens plane with
  /// the direction given by the vector perpendicular to the lens plane.
  /// The lens plane is the plane z = 0 in the camera space, the longer
  /// dimension is given by the dimension parameter;
  class OrthoCamera final: public Camera {
    Vector world_dir;
    float dimension;
  public:
    OrthoCamera(const Transform& camera_to_world, float dimension);

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
    Vec2 get_plane_pos(Vec2 film_res, Vec2 film_pos) const;
    float get_image_plane_area(Vec2 film_res) const;
  };
}

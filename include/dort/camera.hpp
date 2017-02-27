#pragma once
#include "dort/rect.hpp"
#include "dort/sampler.hpp"
#include "dort/transform.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  enum CameraFlags: uint8_t {
    CAMERA_LENS_POS_DELTA = 1,
      ///< For a fixed point on the film, the point on the lens is given by a
      /// delta distribution.
    CAMERA_LENS_DIR_DELTA = 2,
      ///< For a fixed point on the film and given the ray origin on the lens,
      /// the direction of the ray is given by a delta distribution.
    CAMERA_FILM_PIVOT_POS_DELTA = 4,
      ///< For a fixed point in the scene (pivot), the point on the film and the
      /// lens direction is given by a delta distribution.
  };

  struct CameraSamplesIdxs {
    SampleIdx uv_lens_idx;
    uint32_t count;
  };

  struct CameraSample {
    Vec2 uv_lens;

    explicit CameraSample(Rng& rng);
    CameraSample(Sampler& sampler, const CameraSamplesIdxs& idxs, uint32_t n);
    static CameraSamplesIdxs request(Sampler& sampler, uint32_t count);
  };

  /// Cameras map points on the film plane to rays in the scene.
  /// For each point p on the film plane (which has no physical meaning in the
  /// scene), the camera assigns a set of rays (x, wi) on the lens in the scene.
  /// The measurement I(p) of the point p on the film is defined as (the
  /// physical meaning of I is irradiance):
  ///
  /// I(p) = int_(lens area) int_(sphere) We(p, x, wi) Li(x, wi) d(wi) d(x)
  ///
  /// where We(p, x, wi) measures the contribution of ray (x, wi) to the film
  /// point p. This definition of importance is different from the usual
  /// definition, because we define the measurement for points on the continuous
  /// film, not for the discrete pixels.
  class Camera {
  protected:
    Transform camera_to_world;
  public:
    CameraFlags flags;

    explicit Camera(CameraFlags flags, const Transform& camera_to_world):
      camera_to_world(camera_to_world), flags(flags) {}
    virtual ~Camera() {}

    /// Samples a ray from the camera.
    /// Samples a ray emanating from the camera that corresponds to the point
    /// film_pos on the film (with resolution film_res), and returns the
    /// corresponding importance. The pdf of the ray origin (w.r.t. area
    /// measure) and ray direction (given the origin, w.r.t. solid angle from the
    /// origin) is returned in out_pos_pdf and out_dir_pdf, respectively.
    ///
    /// The pdf of the ray origin is a delta distribution if the flag
    /// CAMERA_LENS_DELTA_POS is set, and the pdf of the ray direction (given
    /// the origin) is a delta distribution if the flag CAMERA_LENS_DELTA_DIR is
    /// set.
    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const = 0;

    /// Samples a direction of a ray from the camera through pivot.
    /// The direction out_wo points to the camera and out_shadow is initialized
    /// with an occlusion test. The pdf is w.r.t. the solid angle at pivot.
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Vector& out_wo, float& out_dir_pdf, ShadowTest& out_shadow,
        Vec2& out_film_pos, CameraSample sample) const = 0;

    /// Computes the pdf of sampling ray in the direction, given its origin,
    /// from sample_ray_importance().
    /// Returns the pdf of sampling ray with direction dir_gen, given the ray
    /// origin, from sample_ray_importance() (assuming the film pos is sampled
    /// uniformly on the film plane). The pdf is w.r.t. the solid angle at origin.
    virtual float ray_dir_importance_pdf(Vec2 film_res,
        const Vector& wi_gen, const Point& origin_fix) const = 0;
  protected:
    static Vec2 film_to_normal(Vec2 film_res, Vec2 film_pos);
    static Vec2 normal_to_film(Vec2 film_res, Vec2 normal_pos);
  };

  /// Orthographic camera maps each film point to a point on the lens plane with
  /// the direction given by the vector perpendicular to the lens plane.
  /// The lens plane is the plane z = 0 in the camera space, the longer
  /// dimension is given by the dimension parameter;
  class OrthographicCamera final: public Camera {
    float dimension;
  public:
    OrthographicCamera(const Transform& camera_to_world, float dimension);

    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const override final;
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Vector& out_wo, float& out_dir_pdf, ShadowTest& out_shadow,
        Vec2& out_film_pos, CameraSample sample) const override final;
    virtual float ray_dir_importance_pdf(Vec2 film_res,
        const Vector& wi_gen, const Point& origin_fix) const override final;
  };

  /// Pinhole camera has a pinhole lens at the origin of the camera space and
  /// projects to the plane z = -1. The angle visible in the longer film
  /// dimension is given as the fov parameter;
  class PinholeCamera final: public Camera {
    float project_dimension;
  public:
    PinholeCamera(const Transform& camera_to_world, float fov);

    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const override final;
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Vector& out_wo, float& out_dir_pdf, ShadowTest& out_shadow,
        Vec2& out_film_pos, CameraSample sample) const override final;
    virtual float ray_dir_importance_pdf(Vec2 film_res,
        const Vector& wi_gen, const Point& origin_fix) const override final;
  };

  /*
  /// Thin lens camera projects through a thin lens with depth of field.
  class ThinLensCamera final: public Camera {
    float fov;
    float lens_radius;
    float focal_distance;
  public:
    ThinLensCamera(const Transform& camera_to_world,
        float lens_radius, float focal_distance):
      lens_radius(lens_radius), focal_distance(focal_distance) { }

    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const override final;
    virtual Spectrum sample_pivot_importance(const Point& pivot, float pivot_epsilon,
        Vector& out_wo, float& out_dir_pdf, ShadowTest& out_shadow,
        Vec2& out_film_pos, CameraSample sample) const override final;
  };
  */
}

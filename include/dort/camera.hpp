#pragma once
#include "dort/rect.hpp"
#include "dort/sampler.hpp"
#include "dort/transform.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  enum CameraFlags: uint8_t {
    CAMERA_POS_DELTA = 1,
      ///< The ray origins have delta distribution.
    CAMERA_DIR_DELTA = 2,
      ///< The ray directions have delta distribution.
    CAMERA_DIR_BY_POS_DELTA = 4,
      ///< The ray directions, given ray origin, have delta distribution.
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
    ///
    /// The origin of the ray must correspond to some geometric area in the
    /// scene.
    virtual Spectrum sample_ray_importance(Vec2 film_res, Vec2 film_pos,
        Ray& out_ray, float& out_pos_pdf, float& out_dir_pdf,
        CameraSample sample) const = 0;

    /// Samples a point on camera and on the film that can look at the pivot.
    /// Initializes out_p with a point p on the camera and out_p_pdf with the
    /// pdf value of the point w.r.t. area (conditioned by the pivot). Also
    /// initializes out_film_pos with a sample point on film and out_film_pdf
    /// with the pdf value for this point (conditioned by the pivot and the
    /// point p). Returns the importance from the point p in direction from p to
    /// pivot at the film point.
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Point& out_p, Vec2& out_film_pos,
        float& out_p_pdf, float& out_film_pdf,
        ShadowTest& out_shadow, CameraSample sample) const = 0;

    /// Samples a point on the camera.
    virtual Point sample_point(float& out_pos_pdf, CameraSample sample) const = 0;

    /// Samples a point on film given a point on camera and a direction to look at.
    /// Returns the importance and sets out_film_pos and out_film_pdf.
    virtual Spectrum sample_film_pos(Vec2 film_res,
        const Point& p, const Vector& wi,
        Vec2& out_film_pos, float& out_film_pdf) const = 0;

    /// Computes the pdf of sampling ray from sample_ray_importance(), given a
    /// film_pos. The pdf is a product of area pdf of the origin and solid angle
    /// pdf of the direction.
    virtual float ray_importance_pdf(Vec2 film_res,
        const Point& origin_gen, const Vector& wi_gen, Vec2 film_pos_fix) const = 0;

    /// Computes the area pdf of sampling a point and film pos from
    /// sample_pivot_importance().
    virtual float pivot_importance_pdf(Vec2 film_res,
        const Point& p_gen, Vec2 film_pos_gen, const Point& pivot_fix) const = 0;
  protected:
    static Vec2 film_to_normal(Vec2 film_res, Vec2 film_pos);
    static Vec2 normal_to_film(Vec2 film_res, Vec2 normal_pos);
  };

  /// Orthographic camera maps each film point to a point on the lens plane with
  /// the direction given by the vector perpendicular to the lens plane.
  /// The lens plane is the plane z = 0 in the camera space, the longer
  /// dimension is given by the dimension parameter;
  /*
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
  */

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

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

  /// Cameras map rays in the scene to points on the film plane.
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
    /// origin) is returned in out_pos_pdf and out_dir_pdf, respectively. Note
    /// that the pdfs are not conditioned on the film position; rather, the
    /// film_pos is considered as a random sample.
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

    /// Samples a point on camera that looks at the pivot.
    /// Initializes out_p with a point p on the camera and out_p_pdf with the
    /// pdf value of the point w.r.t. area (conditioned by the pivot). Also
    /// initializes out_film_pos with the film position corresponding to the
    /// point and direction.  Returns the importance.
    virtual Spectrum sample_pivot_importance(Vec2 film_res,
        const Point& pivot, float pivot_epsilon,
        Point& out_p, Vec2& out_film_pos, float& out_p_pdf,
        ShadowTest& out_shadow, CameraSample sample) const = 0;

    /// Samples a point on the camera.
    virtual Point sample_point(float& out_pos_pdf, CameraSample sample) const = 0;

    /// Evaluates the importance from point p in direction wi.
    /// Returns the importance and sets out_film_pos to the film position
    /// corresponding to the point and direction.
    virtual Spectrum eval_importance(Vec2 film_res,
        const Point& p, const Vector& wi, Vec2& out_film_pos) const = 0;

    /// Computes the pdf of sampling ray from sample_ray_importance().
    /// The pdf is a product of area pdf of the origin and solid angle pdf of
    /// the direction.
    virtual float ray_importance_pdf(Vec2 film_res,
        const Point& origin_gen, const Vector& wi_gen) const = 0;

    /// Computes the area pdf of sampling a point on camera from
    /// sample_pivot_importance().
    virtual float pivot_importance_pdf(Vec2 film_res,
        const Point& p_gen, const Point& pivot_fix) const = 0;
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

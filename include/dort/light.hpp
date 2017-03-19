#pragma once
#include "dort/geometry.hpp"
#include "dort/rng.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  enum LightFlags: uint8_t {
    LIGHT_DELTA_POS = 1,
      ///< The origin of rays leaving the light is determined by a discrete
      /// delta distribution.
    LIGHT_DELTA_DIR = 2,
      ///< The direction of rays leaving the light, given the origin, is
      /// determined by a discrete delta distribution.
    LIGHT_AREA = 4,
      ///< Area light that is assigned to a primitive in the scene and can be
      /// intersected by rays.
    LIGHT_BACKGROUND = 8,
      ///< Background light that is implicitly intersected by rays leaving the
      /// scene.
    LIGHT_DISTANT = 16,
      ///< Distant light that has no meaningful distribution of ray origins, but
      /// defines only directional distribution of incoming directions at every
      /// point in the scene.

    LIGHT_DELTA = LIGHT_DELTA_POS | LIGHT_DELTA_DIR,
    LIGHT_ALL = LIGHT_DELTA | LIGHT_AREA | LIGHT_BACKGROUND | LIGHT_DISTANT,
  };

  struct ShadowTest {
    Ray ray;
    bool invisible;

    ShadowTest(): ray(Point(), Vector(), 0.f), invisible(true) { }
    void init_point_point(const Point& p1, float epsilon_1,
        const Point& p2, float epsilon_2);
    void init_point_dir(const Point& pt, float epsilon,
        const Vector& dir);
    void init_invisible();
    bool visible(const Scene& scene) const;
  };

  struct LightSamplesIdxs {
    SampleIdx uv_pos_idx;
    uint32_t count;
  };

  struct LightSample {
    Vec2 uv_pos;

    explicit LightSample(Rng& rng);
    LightSample(Sampler& sampler, const LightSamplesIdxs& idxs, uint32_t n);
    static LightSamplesIdxs request(Sampler& sampler, uint32_t count);
  };

  struct LightRaySamplesIdxs {
    SampleIdx uv_pos_idx;
    SampleIdx uv_dir_idx;
    uint32_t count;
  };

  struct LightRaySample {
    Vec2 uv_pos;
    Vec2 uv_dir;

    LightRaySample(Vec2 uv_pos, Vec2 uv_dir):
      uv_pos(uv_pos), uv_dir(uv_dir) { }
    explicit LightRaySample(Rng& rng);
    LightRaySample(Sampler& sampler, const LightRaySamplesIdxs& idxs, uint32_t n);
    static LightRaySamplesIdxs request(Sampler& sampler, uint32_t count);
  };

  class Light {
  public:
    LightFlags flags;
    uint32_t num_samples;

    Light(LightFlags flags, uint32_t num_samples):
      flags(flags), num_samples(num_samples) { }
    virtual ~Light() {}

    bool matches(LightFlags test) {
      return (this->flags & test) == this->flags;
    }

    /// Samples a ray leaving the light.
    /// The pdf of the generated rays is composed from the pdf of the origin
    /// point (w.r.t. area measure) and the pdf of the direction given the
    /// origin point (w.r.t. solid angle measure). If either of the
    /// distributions contains delta terms, the coefficient of this term is
    /// used as the pdf.
    /// For non-distant lights, the origin lies in the scene.
    /// For distant lights, the ray origin lies out of the scene and out_pos_pdf
    /// is initialized with area pdf on the plane perpendicular to the ray
    /// direction.
    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
        LightRaySample sample) const = 0;

    /// Samples a direction of a ray from light through pivot.
    /// The direction out_wi points to the light and out_shadow is initialized
    /// with an occlusion test. The pdf is w.r.t. the solid angle at pivot.
    /// For non-distant lights, also initializes out_p, out_p_epsilon and out_nn
    /// with the point on the light and the normal here.
    virtual Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, Point& out_p, Normal& out_nn, float& out_p_epsilon,
        float& out_dir_pdf, ShadowTest& out_shadow,
        LightSample sample) const = 0;

    Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, float& out_dir_pdf,
        ShadowTest& out_shadow, LightSample sample) const
    {
      Point dummy_p; Normal dummy_nn; float dummy_epsilon;
      return this->sample_pivot_radiance(pivot, pivot_epsilon,
          out_wi, dummy_p, dummy_nn, dummy_epsilon, 
          out_dir_pdf, out_shadow, sample);
    }

    /// Samples a point on non-distant light.
    /// For distant lights returns false, otherwise initializes out_p, out_nn and
    /// out_pos_pdf. The normal out_nn may be initialized to 0 in case of point
    /// lights.
    virtual bool sample_point(Point& out_p, float& out_p_epsilon,
        Normal& out_nn, float& out_pos_pdf, LightSample sample) const = 0;

    /// Returns the radiance at pivot due to the light from point p with normal n.
    /// This method is applicable only for area lights (in which case p and n
    /// are obtained from the geometric intersection) or for lights that
    /// returned true from sample_point (in this case it is necessary to pass
    /// the point and normal generated by this call).
    virtual Spectrum eval_radiance(const Point& p,
        const Normal& nn, const Point& pivot) const = 0;

    /// Computes the pdf of sampling ray from sample_ray_radiance().
    /// The origin and direction are assumed to be generated by
    /// sample_ray_radiance(), sample_pivot_radiance() or sample_point() (in
    /// this case the direction can be arbitrary). The normal generated by
    /// either of these calls should be passed to this method.
    /// Returns a product of the area pdf of the origin and the solid angle pdf
    /// of the direction.
    virtual float ray_radiance_pdf(const Scene& scene,
        const Point& origin_gen, const Vector& wo_gen, const Normal& nn) const = 0;

    /// Computes the pdf of sampling the direction from pivot in
    /// sample_pivot_radiance.
    /// The pdf is w.r.t. the solid angle at pivot.
    virtual float pivot_radiance_pdf(
        const Vector& wi_gen, const Point& pivot_fix) const = 0;

    /// Computes the radiance that this background light contributes to a ray
    /// leaving the scene. Non-background lights return 0.
    virtual Spectrum background_radiance(const Ray& ray) const = 0;

    /// Approximates the emitted power of this light.
    virtual Spectrum approximate_power(const Scene& scene) const = 0;
  };
}

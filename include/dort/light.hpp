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

    LIGHT_DELTA = LIGHT_DELTA_POS | LIGHT_DELTA_DIR,
    LIGHT_ALL = LIGHT_DELTA | LIGHT_AREA | LIGHT_BACKGROUND,
  };

  struct ShadowTest {
    Ray ray;

    ShadowTest(): ray(Point(), Vector()) { }
    void init_point_point(const Point& p1, float epsilon_1,
        const Point& p2, float epsilon_2);
    void init_point_dir(const Point& pt, float epsilon,
        const Vector& dir);
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
    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pos_pdf, float& out_dir_pdf,
        LightRaySample sample) const = 0;

    /// Samples a direction of a ray from light through pivot.
    /// The direction out_wi points to the light and out_shadow is initialized
    /// with an occlusion test. The pdf is w.r.t. the solid angle at pivot.
    virtual Spectrum sample_pivot_radiance(const Point& pivot, float pivot_epsilon,
        Vector& out_wi, float& out_dir_pdf, ShadowTest& out_shadow, 
        LightSample sample) const = 0;

    /// Computes the pdf of sampling the direction from pivot in
    /// sample_pivot_radiance.
    /// The pdf is w.r.t. the solid angle at pivot.
    virtual float pivot_radiance_pdf(const Point& pivot, const Vector& wi) const = 0;

    /// Computes the radiance that this background light contributes to a ray
    /// leaving the scene. Non-background lights return 0.
    virtual Spectrum background_radiance(const Ray& ray) const = 0;

    /// Approximates the emitted power of this light.
    virtual Spectrum approximate_power(const Scene& scene) const = 0;
  };

  class AreaLight: public Light {
  public:
    AreaLight(LightFlags flags, uint32_t num_samples):
      Light(flags, num_samples) { }

    /// Returns the radiance emitted by the light from point pt with normal n in
    /// direction wo.
    virtual Spectrum emitted_radiance(const Point& pt,
        const Normal& n, const Vector& wo) const = 0;
  };
}

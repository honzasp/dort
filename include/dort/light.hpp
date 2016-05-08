#pragma once
#include "dort/geometry.hpp"
#include "dort/rng.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  enum LightFlags: uint8_t {
    LIGHT_DELTA = 1 << 0,
    LIGHT_AREA = 1 << 1,
    LIGHT_BACKGROUND = 1 << 2,
  };
  constexpr LightFlags LIGHT_ALL = LightFlags(
      LIGHT_DELTA | LIGHT_AREA | LIGHT_BACKGROUND);

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

    explicit LightSample(Sampler& sampler);
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
    explicit LightRaySample(Sampler& sampler);
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

    virtual Spectrum sample_ray_radiance(const Scene& scene, 
        Ray& out_ray, Normal& out_nn, float& out_pdf,
        LightRaySample sample) const = 0;
    virtual Spectrum sample_radiance(const Point& eye, float eye_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow, 
        LightSample sample) const = 0;
    virtual float radiance_pdf(const Point& eye, const Vector& wi) const = 0;
    virtual Spectrum background_radiance(const Ray& ray) const = 0;
    virtual Spectrum approximate_power(const Scene& scene) const = 0;
  };

  class AreaLight: public Light {
  public:
    AreaLight(LightFlags flags, uint32_t num_samples):
      Light(flags, num_samples) { }

    virtual Spectrum emitted_radiance(const Point& pt,
        const Normal& n, const Vector& wo) const = 0;
  };
}

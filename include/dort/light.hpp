#pragma once
#include "dort/geometry.hpp"
#include "dort/rng.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"

namespace dort {
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

  class Light {
  public:
    uint32_t num_samples;

    Light(uint32_t num_samples): num_samples(num_samples) { }
    virtual ~Light() {}

    virtual Spectrum sample_radiance(const Point& eye, float eye_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow, 
        LightSample sample) const = 0;
    virtual float radiance_pdf(const Point& pt, const Vector& wi) const = 0;
    virtual Spectrum background_radiance(const Ray& ray) const = 0;
    virtual bool is_delta() const = 0;
  };

  class AreaLight: public Light {
  public:
    AreaLight(uint32_t num_samples): Light(num_samples) { }

    virtual Spectrum emitted_radiance(const Point& pt,
        const Normal& n, const Vector& wo) const = 0;
  };
}

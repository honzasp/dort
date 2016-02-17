#pragma once
#include "dort/rect.hpp"
#include "dort/rng.hpp"
#include "dort/slice.hpp"

namespace dort {
  struct SampleIdx {
    uint32_t i;

    SampleIdx(): i(-1u) { }
    explicit constexpr SampleIdx(uint32_t i): i(i) { }

    bool operator==(SampleIdx idx) { return this->i == idx.i; }
    bool operator!=(SampleIdx idx) { return this->i != idx.i; }
  };

  class Sampler {
    uint32_t requests_1d;
    uint32_t requests_2d;
    std::vector<uint32_t> requests_array_1d;
    std::vector<uint32_t> requests_array_2d;
  protected:
    Rng rng;
    std::vector<float> samples_1d;
    std::vector<Vec2> samples_2d;
    std::vector<std::vector<float>> arrays_1d;
    std::vector<std::vector<Vec2>> arrays_2d;
  public:
    uint32_t samples_per_pixel;
  protected:
    Sampler(uint32_t samples_per_pixel, Rng rng):
      requests_1d(0), requests_2d(0),
      rng(std::move(rng)),
      samples_per_pixel(samples_per_pixel) { }
    Sampler(const Sampler& parent, Rng rng);
  public:
    virtual ~Sampler() { }
    virtual void next_pixel() = 0;
    virtual void next_pixel_sample() = 0;
    virtual std::shared_ptr<Sampler> split() = 0;
    virtual uint32_t round_count(uint32_t count) {
      return count; 
    }

    float get_sample_1d(SampleIdx idx) const {
      return this->samples_1d.at(idx.i);
    }
    Vec2 get_sample_2d(SampleIdx idx) const {
      return this->samples_2d.at(idx.i);
    }
    slice<const float> get_array_1d(SampleIdx idx) const {
      return make_slice(this->arrays_1d.at(idx.i));
    }
    slice<const Vec2> get_array_2d(SampleIdx idx) const {
      return make_slice(this->arrays_2d.at(idx.i));
    }

    SampleIdx request_sample_1d();
    SampleIdx request_sample_2d();
    SampleIdx request_array_1d(uint32_t count);
    SampleIdx request_array_2d(uint32_t count);

    float random_1d() {
      return this->rng.uniform_float();
    }
    Vec2 random_2d() {
      return Vec2(this->rng.uniform_float(), this->rng.uniform_float());
    }
  };
}

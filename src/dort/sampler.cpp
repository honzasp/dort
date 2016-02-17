#include "dort/sampler.hpp"

namespace dort {
  Sampler::Sampler(const Sampler& parent, Rng rng):
    requests_1d(parent.requests_1d),
    requests_2d(parent.requests_2d),
    requests_array_1d(parent.requests_array_1d),
    requests_array_2d(parent.requests_array_2d),
    rng(std::move(rng)),
    samples_1d(parent.samples_1d),
    samples_2d(parent.samples_2d),
    arrays_1d(parent.arrays_1d),
    arrays_2d(parent.arrays_2d),
    samples_per_pixel(parent.samples_per_pixel)
  { }

  SampleIdx Sampler::request_sample_1d() {
    this->requests_1d += 1;
    this->samples_1d.push_back(SIGNALING_NAN);
    return SampleIdx(this->samples_1d.size() - 1);
  }

  SampleIdx Sampler::request_sample_2d() {
    this->requests_2d += 1;
    this->samples_2d.push_back(Vec2(SIGNALING_NAN, SIGNALING_NAN));
    return SampleIdx(this->samples_2d.size() - 1);
  }

  SampleIdx Sampler::request_array_1d(uint32_t count) {
    assert(count == this->round_count(count));
    this->requests_array_1d.push_back(count);
    this->arrays_1d.push_back(std::vector<float>(count, SIGNALING_NAN));
    return SampleIdx(this->arrays_1d.size() - 1);
  }

  SampleIdx Sampler::request_array_2d(uint32_t count) {
    assert(count == this->round_count(count));
    this->requests_array_2d.push_back(count);
    this->arrays_2d.push_back(std::vector<Vec2>(count,
        Vec2(SIGNALING_NAN, SIGNALING_NAN)));
    return SampleIdx(this->arrays_2d.size() - 1);
  }
}

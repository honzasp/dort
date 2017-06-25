#include "dort/random_sampler.hpp"

namespace dort {
  RandomSampler::RandomSampler(const RandomSampler& parent, Rng rng):
    Sampler(parent, std::move(rng))
  { }

  void RandomSampler::start_pixel() {
  }

  void RandomSampler::start_pixel_sample() {
    for(float& x: this->samples_1d) {
      x = this->random_1d();
    }
    for(Vec2& v: this->samples_2d) {
      v = this->random_2d();
    }
    for(std::vector<float>& xs: this->arrays_1d) {
      for(float& x: xs) {
        x = this->random_1d();
      }
    }
    for(std::vector<Vec2>& vs: this->arrays_2d) {
      for(Vec2& v: vs) {
        v = this->random_2d();
      }
    }
  }

  std::shared_ptr<Sampler> RandomSampler::split(uint32_t seed) {
    return std::make_shared<RandomSampler>(*this, Rng(seed));
  }
}

#pragma once
#include "dort/rng.hpp"
#include "dort/sampler.hpp"

namespace dort {
  class RandomSampler final: public Sampler {
  public:
    RandomSampler(const RandomSampler& parent, Rng rng);
    RandomSampler(uint32_t samples_per_pixel, Rng rng):
      Sampler(samples_per_pixel, std::move(rng)) { }

    virtual void start_pixel() override final;
    virtual void start_pixel_sample() override final;
    virtual std::shared_ptr<Sampler> split(uint32_t seed) override final;
  };
}

#pragma once
#include "dort/rng.hpp"
#include "dort/sampler.hpp"

namespace dort {
  class RandomSampler final: public Sampler {
  public:
    RandomSampler(const RandomSampler& parent, Rng rng);
    RandomSampler(uint32_t samples_per_pixel, Rng rng):
      Sampler(samples_per_pixel, std::move(rng)) { }

    virtual void next_pixel() override final;
    virtual void next_pixel_sample() override final;
    virtual std::shared_ptr<Sampler> split() override final;
  };
}

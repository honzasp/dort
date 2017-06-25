#pragma once
#include "dort/sampler.hpp"

namespace dort {
  class StratifiedSampler final: public Sampler {
    std::vector<std::vector<float>> sample_strata_1d;
    std::vector<std::vector<Vec2>> sample_strata_2d;
    uint32_t samples_per_x;
    uint32_t samples_per_y;
    uint32_t current_pixel_sample_i;
  public:
    StratifiedSampler(uint32_t samples_per_x, uint32_t samples_per_y, Rng rng);
    StratifiedSampler(const StratifiedSampler& parent, Rng rng);
    virtual void start_pixel() override final;
    virtual void start_pixel_sample() override final;
    virtual std::shared_ptr<Sampler> split(uint32_t seed) override final;

    virtual SampleIdx request_sample_1d() override;
    virtual SampleIdx request_sample_2d() override;
  };
}

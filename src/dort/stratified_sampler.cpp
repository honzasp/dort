#include "dort/monte_carlo.hpp"
#include "dort/stratified_sampler.hpp"

namespace dort {
  StratifiedSampler::StratifiedSampler(
      uint32_t samples_per_x, uint32_t samples_per_y, Rng rng):
    Sampler(samples_per_x * samples_per_y, std::move(rng)),
    samples_per_x(samples_per_x),
    samples_per_y(samples_per_y)
  { }

  StratifiedSampler::StratifiedSampler(const StratifiedSampler& parent, Rng rng):
    Sampler(parent, std::move(rng)),
    sample_strata_1d(parent.sample_strata_1d),
    sample_strata_2d(parent.sample_strata_2d),
    samples_per_x(parent.samples_per_x),
    samples_per_y(parent.samples_per_y),
    current_pixel_sample_i(parent.current_pixel_sample_i)
  { }

  void StratifiedSampler::start_pixel() {
    for(uint32_t i = 0; i < this->samples_1d.size(); ++i) {
      stratified_sample(make_slice(this->sample_strata_1d.at(i)), this->rng);
      shuffle(make_slice(this->sample_strata_1d.at(i)), this->rng);
    }

    for(uint32_t i = 0; i < this->samples_2d.size(); ++i) {
      stratified_sample(make_slice(this->sample_strata_2d.at(i)), 
          this->samples_per_x, this->samples_per_y, this->rng);
    }

    this->current_pixel_sample_i = -1u;
  }

  void StratifiedSampler::start_pixel_sample() {
    this->current_pixel_sample_i += 1;
    assert(this->current_pixel_sample_i < this->samples_per_pixel);

    for(uint32_t i = 0; i < this->samples_1d.size(); ++i) {
      this->samples_1d.at(i) = this->sample_strata_1d.at(i)
          .at(this->current_pixel_sample_i);
    }
    for(uint32_t i = 0; i < this->samples_2d.size(); ++i) {
      this->samples_2d.at(i) = this->sample_strata_2d.at(i)
          .at(this->current_pixel_sample_i);
    }

    for(auto& array: this->arrays_1d) {
      stratified_sample(make_slice(array), this->rng);
    }
    for(auto& array: this->arrays_2d) {
      latin_hypercube_sample(make_slice(array), this->rng);
    }
  }

  std::shared_ptr<Sampler> StratifiedSampler::split() {
    return std::make_shared<StratifiedSampler>(*this, this->rng.split());
  }

  SampleIdx StratifiedSampler::request_sample_1d() {
    this->sample_strata_1d.push_back(std::vector<float>(
          this->samples_per_pixel, SIGNALING_NAN));
    return Sampler::request_sample_1d();
  }

  SampleIdx StratifiedSampler::request_sample_2d() {
    this->sample_strata_2d.push_back(std::vector<Vec2>(
          this->samples_per_pixel, Vec2(SIGNALING_NAN, SIGNALING_NAN)));
    return Sampler::request_sample_2d();
  }
}

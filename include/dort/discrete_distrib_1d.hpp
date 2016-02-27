#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  class DiscreteDistrib1d {
    std::vector<float> cdf;
    float sum_;
  public:
    DiscreteDistrib1d() = default;
    DiscreteDistrib1d(const std::vector<float>& xs);
    uint32_t sample(float u) const;
    float pdf(uint32_t sample) const;
    float sum() const { return this->sum_; }
  };
}

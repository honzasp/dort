#include "dort/discrete_distrib_1d.hpp"

namespace dort {
  DiscreteDistrib1d::DiscreteDistrib1d(const std::vector<float>& xs) {
    this->cdf.reserve(xs.size() + 1);

    float sum = 0.f;
    for(uint32_t i = 0; i < xs.size(); ++i) {
      assert(xs.at(i) >= 0.f);
      this->cdf.push_back(sum);
      sum += xs.at(i);
    }
    this->cdf.push_back(sum);

    this->sum_ = sum;
    float inv_sum = sum == 0.f ? 0.f : 1.f / sum;
    for(uint32_t i = 0; i <= xs.size(); ++i) {
      this->cdf.at(i) *= inv_sum;
    }
  }

  uint32_t DiscreteDistrib1d::sample(float u) const {
    assert(u >= 0.f && u <= 1.f);
    uint32_t begin = 0;
    uint32_t end = this->cdf.size() - 1;
    while(begin + 1 < end) {
      uint32_t mid = begin + (end - begin) / 2;
      if(this->cdf.at(mid) < u) {
        begin = mid;
      } else {
        end = mid;
      }
    }
    return begin;
  }

  float DiscreteDistrib1d::pdf(uint32_t sample) const {
    if(sample + 1 >= this->cdf.size()) {
      return 0.f;
    }
    return this->cdf.at(sample + 1) - this->cdf.at(sample);
  }
}

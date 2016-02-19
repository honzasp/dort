#pragma once
#include <random>
#include "dort/stats.hpp"

namespace dort {
  class Rng {
    std::mt19937 gen;
  public:
    Rng(uint32_t seed): gen(seed) { }
    Rng(const Rng&) = delete;
    Rng(Rng&&) = default;
    Rng& operator=(const Rng&) = delete;
    Rng& operator=(Rng&&) = default;

    float uniform_float() {
      //StatTimer t(TIMER_RNG_FLOAT);
      return std::uniform_real_distribution<float>(0.f, 1.f)(this->gen);
    }

    uint32_t uniform_uint32(uint32_t limit) {
      //StatTimer t(TIMER_RNG_UINT32);
      return std::uniform_int_distribution<uint32_t>(0u, limit - 1)(this->gen);
    }

    Rng split() {
      return Rng(this->gen());
    }
  };
}

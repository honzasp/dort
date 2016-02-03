#pragma once
#include <random>

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
      return std::uniform_real_distribution<float>(0.f, 1.f)(this->gen);
    }
  };
}

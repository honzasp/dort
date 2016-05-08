#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  std::vector<float> low_discrepancy_1d(uint32_t chunk_count,
      uint32_t chunk_size, Rng& rng);
  std::vector<Vec2> low_discrepancy_2d(uint32_t chunk_count,
      uint32_t chunk_size, Rng& rng);

  float van_der_corput(uint32_t x, uint32_t scramble);
  float sobol2(uint32_t x, uint32_t scramble);
  Vec2 zero_two(uint32_t x, uint32_t scramble0, uint32_t scramble1);
}

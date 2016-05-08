#include "dort/low_discrepancy.hpp"
#include "dort/monte_carlo.hpp"
#include "dort/rng.hpp"

namespace dort {
  std::vector<float> low_discrepancy_1d(uint32_t chunk_count,
      uint32_t chunk_size, Rng& rng)
  {
    uint32_t scramble = rng.uniform_uint32(0xffffffff);
    std::vector<float> samples(chunk_count * chunk_size);
    for(uint32_t i = 0; i < chunk_count * chunk_size; ++i) {
      samples.at(i) = van_der_corput(i, scramble);
    }
    for(uint32_t i = 0; i < chunk_count; ++i) {
      shuffle(slice<float>(samples).subslice_len(i * chunk_size, chunk_size), rng);
    }
    shuffle_chunks(slice<float>(samples), chunk_size, rng);
    return samples;
  }

  std::vector<Vec2> low_discrepancy_2d(uint32_t chunk_count,
      uint32_t chunk_size, Rng& rng)
  {
    uint32_t scramble0 = rng.uniform_uint32(0xffffffff);
    uint32_t scramble1 = rng.uniform_uint32(0xffffffff);
    std::vector<Vec2> samples(chunk_count * chunk_size);
    for(uint32_t i = 0; i < chunk_count * chunk_size; ++i) {
      samples.at(i) = zero_two(i, scramble0, scramble1);
    }
    for(uint32_t i = 0; i < chunk_count; ++i) {
      shuffle(slice<Vec2>(samples).subslice_len(i * chunk_size, chunk_size), rng);
    }
    shuffle_chunks(slice<Vec2>(samples), chunk_size, rng);
    return samples;
  }

  float van_der_corput(uint32_t x, uint32_t scramble) {
    x = reverse_bits(x) ^ scramble;
    return (x >> 8) * (1.f / float(1 << 24));
  }

  float sobol2(uint32_t x, uint32_t scramble) {
    uint32_t v = 1 << 31;
    uint32_t y = scramble;
    while(x != 0) {
      y = (x & 1) ? y ^ v : y;
      x = x >> 1;
      v = v ^ (v >> 1);
    }
    return (y >> 8) * (1.f / float(1 << 24));
  }

  Vec2 zero_two(uint32_t x, uint32_t scramble0, uint32_t scramble1) {
    return Vec2(van_der_corput(x, scramble0), sobol2(x, scramble1));
  }
}

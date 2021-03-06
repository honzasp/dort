#pragma once
#include <atomic>

namespace dort {
  class atomic_float {
    std::atomic<uint32_t> bits;

    static float bits_to_float(uint32_t bits) {
      union { float f; uint32_t b; } bitcast;
      bitcast.b = bits; return bitcast.f;
    }
    static uint32_t float_to_bits(float f) {
      union { float f; uint32_t b; } bitcast;
      bitcast.f = f; return bitcast.b;
    }
  public:
    atomic_float(float value = 0.f):
      bits(float_to_bits(value)) { }
    void store(float val, std::memory_order order = std::memory_order_seq_cst) {
      this->bits.store(float_to_bits(val), order);
    }
    float load(std::memory_order order = std::memory_order_seq_cst) const {
      return bits_to_float(this->bits.load(order));
    }
    void add_relaxed(float val) {
      uint32_t observed = this->bits.load(std::memory_order_relaxed);
      float sum;
      do {
        sum = bits_to_float(observed) + val;
      } while(!this->bits.compare_exchange_weak(observed,
          float_to_bits(sum), std::memory_order_relaxed));
    }
  };
}

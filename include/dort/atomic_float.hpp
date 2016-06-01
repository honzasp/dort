#pragma once
#include <atomic>
#include <cstdint>

namespace dort {
  class atomic_float {
    union Bitcast {
      uint32_t bits;
      float flt;
    };
    static float btf(uint32_t b) {
      Bitcast cast; cast.bits = b; return cast.flt;
    }
    static float ftb(float f) {
      Bitcast cast; cast.flt = f; return cast.bits;
    }

    std::atomic<uint32_t> bits;

    template<class F>
    void apply(F fun) {
      for(;;) {
        uint32_t current = this->bits.load();
        uint32_t desired = ftb(fun(btf(current)));
        if(this->bits.compare_exchange_weak(current, desired)) {
          break;
        }
      }
    }
  public:
    atomic_float(): atomic_float(0.f) { }
    atomic_float(float a): bits(ftb(a)) { }
    atomic_float(const atomic_float&) = delete;
    atomic_float(atomic_float&&) = delete;
    atomic_float& operator=(const atomic_float&) = delete;

    float load() const {
      return btf(this->bits.load());
    }
    void store(float x) {
      this->bits.store(ftb(x));
    }

    void add(float b) {
      this->apply([&](float a) { return a + b; });
    }
    void sub(float b) {
      this->apply([&](float a) { return a - b; });
    }
    void mul(float b) {
      this->apply([&](float a) { return a * b; });
    }
  };
}

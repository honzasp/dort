#pragma once
#include "dort/atomic_float.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  struct AtomicRgbSpectrum {
    atomic_float r, g, b;

    AtomicRgbSpectrum(): r(0.f), g(0.f), b(0.f) { }
    explicit AtomicRgbSpectrum(RgbSpectrum rgb):
      r(rgb.red()), g(rgb.green()), b(rgb.blue()) { }

    RgbSpectrum load_relaxed() const {
      float r = this->r.load(std::memory_order_relaxed);
      float g = this->g.load(std::memory_order_relaxed);
      float b = this->b.load(std::memory_order_relaxed);
      return RgbSpectrum(r, g, b);
    }

    void store_relaxed(RgbSpectrum rgb) {
      this->r.store(rgb.red(), std::memory_order_relaxed);
      this->g.store(rgb.green(), std::memory_order_relaxed);
      this->b.store(rgb.blue(), std::memory_order_relaxed);
    }

    void add_relaxed(RgbSpectrum rgb) {
      this->r.add_relaxed(rgb.red());
      this->g.add_relaxed(rgb.green());
      this->b.add_relaxed(rgb.blue());
    }
  };
}



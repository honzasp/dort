#pragma once
#include <array>
#include "dort/atomic_float.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  struct AtomicSpectrum {
    std::array<atomic_float, Spectrum::SAMPLES> samples;

    AtomicSpectrum() = default;
    explicit AtomicSpectrum(const Spectrum& spectrum) {
      for(uint32_t i = 0; i < Spectrum::SAMPLES; ++i) {
        this->samples.at(i).store(spectrum.sample(i));
      }
    }

    Spectrum load() const {
      Spectrum spectrum;
      for(uint32_t i = 0; i < Spectrum::SAMPLES; ++i) {
        spectrum.sample(i, this->samples.at(i).load());
      }
      return spectrum;
    }


    void add(const Spectrum& s1) {
      for(uint32_t i = 0; i < Spectrum::SAMPLES; ++i) {
        this->samples.at(i).add(s1.sample(i));
      }
    }

    void sub(const Spectrum& s1) {
      this->add(-s1);
    }

    void mul(const Spectrum& s1) {
      for(uint32_t i = 0; i < Spectrum::SAMPLES; ++i) {
        this->samples.at(i).mul(s1.sample(i));
      }
    }

    void mul(float a) {
      for(uint32_t i = 0; i < Spectrum::SAMPLES; ++i) {
        this->samples.at(i).mul(a);
      }
    }
  };
}


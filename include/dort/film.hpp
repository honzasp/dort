#pragma once
#include <cstdio>
#include <vector>
#include "dort/spectrum.hpp"

namespace dort {
  struct Film {
    struct Pixel {
      Spectrum color;
      float weight;
    };

    uint32_t width;
    uint32_t height;
    std::vector<Pixel> pixels;

    Film(uint32_t width, uint32_t height);
    void add_sample(float x, float y, const Spectrum& radiance);
    void write_ppm(FILE* output) const;
    uint32_t pixel_idx(int32_t x, int32_t y) const {
      assert(x >= 0 && x < int32_t(this->width));
      assert(y >= 0 && y < int32_t(this->height));
      return this->width * y + x;
    }
  };
}

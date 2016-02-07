#pragma once
#include <vector>
#include "dort/image.hpp"

namespace dort {
  struct Film {
    struct Pixel {
      Spectrum color;
      float weight;
    };

    uint32_t x_res;
    uint32_t y_res;
    std::vector<Pixel> pixels;

    Film(uint32_t x_res, uint32_t y_res);
    void add_sample(float x, float y, const Spectrum& radiance);
    Image<RgbPixel8> to_image() const;

    uint32_t pixel_idx(int32_t x, int32_t y) const {
      assert(x >= 0 && x < int32_t(this->x_res));
      assert(y >= 0 && y < int32_t(this->y_res));
      return this->x_res * y + x;
    }
  };
}

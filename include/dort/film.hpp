#pragma once
#include <vector>
#include "dort/image.hpp"
#include "dort/vec_2.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  struct Film {
    struct Pixel {
      Spectrum color;
      float weight;
    };

    uint32_t x_res;
    uint32_t y_res;
    std::vector<Pixel> pixels;
    std::shared_ptr<Filter> filter;

    Film(uint32_t x_res, uint32_t y_res, std::shared_ptr<Filter> filter);
    void add_sample(Vec2 pos, const Spectrum& radiance);
    void add_tile(Vec2i pos, const Film& tile);
    Image<PixelRgb8> to_image() const;

    uint32_t pixel_idx(int32_t x, int32_t y) const {
      assert(x >= 0 && x < int32_t(this->x_res));
      assert(y >= 0 && y < int32_t(this->y_res));
      return this->x_res * y + x;
    }
  };
}

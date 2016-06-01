#pragma once
#include "dort/atomic_float.hpp"
#include "dort/atomic_spectrum.hpp"
#include "dort/film.hpp"

namespace dort {
  struct AtomicFilm {
    struct Pixel {
      AtomicSpectrum color;
      atomic_float weight;
    };

    uint32_t x_res;
    uint32_t y_res;
    std::vector<Pixel> pixels;

    AtomicFilm(uint32_t x_res, uint32_t y_res);
    Film into_film(SampledFilter filter) &&;
    void add_tile(Vec2i pos, const Film& tile);

    uint32_t pixel_idx(int32_t x, int32_t y) const {
      assert(x >= 0 && x < int32_t(this->x_res));
      assert(y >= 0 && y < int32_t(this->y_res));
      return this->x_res * y + x;
    }
  };
}


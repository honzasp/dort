#include "dort/atomic_film.hpp"

namespace dort {
  AtomicFilm::AtomicFilm(uint32_t x_res, uint32_t y_res):
    x_res(x_res), y_res(y_res),
    pixels(x_res * y_res)
  { }

  Film AtomicFilm::into_film(SampledFilter filter) && {
    Film film(this->x_res, this->y_res, std::move(filter));
    for(uint32_t i = 0; i < this->x_res * this->y_res; ++i) {
      Film::Pixel& dst_pixel = film.pixels.at(i);
      const AtomicFilm::Pixel& src_pixel = this->pixels.at(i);
      dst_pixel.weight = src_pixel.weight.load();
      dst_pixel.color = src_pixel.color.load();
    }
    return film;
  }

  void AtomicFilm::add_tile(Vec2i pos, const Film& tile) {
    uint32_t y_min = max(0, -pos.y);
    uint32_t x_min = max(0, -pos.x);
    uint32_t y_max = min(tile.y_res, this->y_res - pos.y);
    uint32_t x_max = min(tile.x_res, this->x_res - pos.x);

    for(uint32_t y = y_min; y < y_max; ++y) {
      for(uint32_t x = x_min; x < x_max; ++x) {
        uint32_t this_idx = this->pixel_idx(pos.x + x, pos.y + y);
        AtomicFilm::Pixel& this_pixel = this->pixels.at(this_idx);
        uint32_t tile_idx = tile.pixel_idx(x, y);
        const Film::Pixel& tile_pixel = tile.pixels.at(tile_idx);
        this_pixel.color.add(tile_pixel.color);
        this_pixel.weight.add(tile_pixel.weight);
      }
    }
  }
}

#include "dort/film.hpp"
#include "dort/filter.hpp"
#include "dort/stats.hpp"

namespace dort {
  Film::Film(uint32_t x_res, uint32_t y_res, std::shared_ptr<Filter> filter):
    Film(x_res, y_res, SampledFilter(filter, Vec2i(12, 12)))
  { }

  Film::Film(uint32_t x_res, uint32_t y_res, SampledFilter filter):
    x_res(x_res), y_res(y_res), pixels(x_res * y_res),
    filter(std::move(filter))
  { }

  void Film::add_sample(Vec2 pos, const Spectrum& radiance) {
    StatTimer t(TIMER_FILM_ADD_SAMPLE);

    int32_t x0 = ceil_int32(pos.x - 0.5f - this->filter.radius.x);
    int32_t x1 = floor_int32(pos.x - 0.5f + this->filter.radius.x);
    int32_t y0 = ceil_int32(pos.y - 0.5f - this->filter.radius.y);
    int32_t y1 = floor_int32(pos.y - 0.5f + this->filter.radius.y);

    int32_t pix_x0 = max(0, x0);
    int32_t pix_y0 = max(0, y0);
    int32_t pix_x1 = min(int32_t(this->x_res) - 1, x1);
    int32_t pix_y1 = min(int32_t(this->y_res) - 1, y1);

    for(int32_t pix_y = pix_y0; pix_y <= pix_y1; ++pix_y) {
      for(int32_t pix_x = pix_x0; pix_x <= pix_x1; ++pix_x) {
        Vec2 filter_p(float(pix_x) + 0.5f - pos.x, float(pix_y) + 0.5f - pos.y);
        float filter_w = this->filter.evaluate(filter_p);
        Film::Pixel& pixel = this->pixels.at(this->pixel_idx(pix_x, pix_y));
        pixel.color += radiance * filter_w;
        pixel.weight += filter_w;
      }
    }
  }

  void Film::add_tile(Vec2i pos, const Film& tile) {
    StatTimer t(TIMER_FILM_ADD_TILE);

    uint32_t y_min = max(0, -pos.y);
    uint32_t x_min = max(0, -pos.x);
    uint32_t y_max = min(tile.y_res, this->y_res - pos.y);
    uint32_t x_max = min(tile.x_res, this->x_res - pos.x);

    for(uint32_t y = y_min; y < y_max; ++y) {
      for(uint32_t x = x_min; x < x_max; ++x) {
        uint32_t this_idx = this->pixel_idx(pos.x + x, pos.y + y);
        Film::Pixel& this_pixel = this->pixels.at(this_idx);
        uint32_t tile_idx = tile.pixel_idx(x, y);
        const Film::Pixel& tile_pixel = tile.pixels.at(tile_idx);
        this_pixel.color += tile_pixel.color;
        this_pixel.weight += tile_pixel.weight;
      }
    }
  }

  Image<PixelRgb8> Film::to_image() const {
    Image<PixelRgb8> img(this->x_res, this->y_res);
    for(uint32_t y = 0; y < this->y_res; ++y) {
      for(uint32_t x = 0; x < this->x_res; ++x) {
        const Film::Pixel& pixel = this->pixels.at(this->pixel_idx(x, y));
        Spectrum color = pixel.weight != 0.f ? pixel.color / pixel.weight : Spectrum();
        img.set_rgb(x, y, color);
      }
    }
    return img;
  }

}

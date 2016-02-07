#include "dort/film.hpp"

namespace dort {
  Film::Film(uint32_t x_res, uint32_t y_res):
    x_res(x_res), y_res(y_res), pixels(x_res * y_res)
  { }

  void Film::add_sample(float x, float y, const Spectrum& radiance) {
    int32_t pixel_x = floor_int32(x);
    int32_t pixel_y = floor_int32(y);
    Film::Pixel& pixel = this->pixels.at(this->pixel_idx(pixel_x, pixel_y));
    pixel.color = pixel.color + radiance;
    pixel.weight = pixel.weight + 1.f;
  }

  Image<RgbPixel8> Film::to_image() const {
    Image<RgbPixel8> img(this->x_res, this->y_res);
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

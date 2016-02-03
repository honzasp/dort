#include "dort/film.hpp"

namespace dort {
  Film::Film(uint32_t width, uint32_t height):
    width(width), height(height), pixels(width * height)
  { }

  void Film::add_sample(float x, float y, const Spectrum& radiance) {
    int32_t pixel_x = floor_int32(x);
    int32_t pixel_y = floor_int32(y);
    Film::Pixel& pixel = this->pixels.at(this->pixel_idx(pixel_x, pixel_y));
    pixel.color = pixel.color + radiance;
    pixel.weight = pixel.weight + 1.f;
  }

  void Film::write_ppm(FILE* output) const {
    std::fprintf(output, "P6 %u %u 255\n", this->width, this->height);
    for(uint32_t y = 0; y < height; ++y) {
      for(uint32_t x = 0; x < width; ++x) {
        const Film::Pixel& pixel = this->pixels.at(this->pixel_idx(x, y));
        Spectrum color = pixel.weight != 0.f ? pixel.color / pixel.weight : Spectrum();

        uint8_t r_level = clamp(floor_int32(256.f * color.red()), 0, 255);
        uint8_t g_level = clamp(floor_int32(256.f * color.green()), 0, 255);
        uint8_t b_level = clamp(floor_int32(256.f * color.blue()), 0, 255);
        uint8_t rgb[3] = { r_level, g_level, b_level };
        std::fwrite(rgb, 3, 1, output);
      }
    }
  }
}

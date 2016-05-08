#pragma once
#include <cstdio>
#include <vector>
#include "dort/spectrum.hpp"

namespace dort {
  template<class Pixel>
  struct Image {
    uint32_t x_res;
    uint32_t y_res;
    std::vector<typename Pixel::StorageT> storage;

    Image(uint32_t x_res, uint32_t y_res):
      x_res(x_res), y_res(y_res),
      storage(x_res * y_res * Pixel::storage_size)
    { }

    explicit Image(const Image&) = default;
    Image(Image&&) = default;

    uint32_t index(uint32_t x, uint32_t y) const {
      assert(x < this->x_res && y < this->y_res);
      return y * this->x_res + x;
    }

    Pixel get_pixel(uint32_t x, uint32_t y) const {
      uint32_t offset = Pixel::storage_size * this->index(x, y);
      return Pixel::read_storage(this->storage.data() + offset);
    }
    void set_pixel(uint32_t x, uint32_t y, const Pixel& pixel) {
      uint32_t offset = Pixel::storage_size * this->index(x, y);
      Pixel::write_storage(this->storage.data() + offset, pixel);
    }

    RgbSpectrum get_rgb(uint32_t x, uint32_t y) const {
      return Pixel::to_rgb(this->get_pixel(x, y));
    }
    void set_rgb(uint32_t x, uint32_t y, const RgbSpectrum& rgb) {
      this->set_pixel(x, y, Pixel::from_rgb(rgb));
    }
  };

  struct PixelRgb8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    PixelRgb8(): r(0), g(0), b(0) { }
    PixelRgb8(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) { }

    using StorageT = uint8_t;
    static constexpr uint32_t storage_size = 3;
    static PixelRgb8 read_storage(const uint8_t* data) {
      return PixelRgb8(data[0], data[1], data[2]);
    }
    static void write_storage(uint8_t* data, PixelRgb8 pix) {
      data[0] = pix.r; data[1] = pix.g; data[2] = pix.b;
    }

    static PixelRgb8 from_rgb(const RgbSpectrum& spectrum) {
      auto map = [](float chan) {
        return clamp(floor_int32(256.f * chan), 0, 255);
      };
      return PixelRgb8(map(spectrum.rgb.x), map(spectrum.rgb.y), map(spectrum.rgb.z));
    }
    static RgbSpectrum to_rgb(PixelRgb8 pix) {
      auto map = [](uint8_t chan) {
        return mul_power_of_two(float(chan), -8);
      };
      return RgbSpectrum(map(pix.r), map(pix.g), map(pix.b));
    }
  };

  struct PixelRgbFloat {
    float r;
    float g;
    float b;

    PixelRgbFloat(): r(0.f), g(0.f), b(0.f) { }
    PixelRgbFloat(float r, float g, float b): r(r), g(g), b(b) { }

    using StorageT = float;
    static constexpr uint32_t storage_size = 3;
    static PixelRgbFloat read_storage(const float* data) {
      return PixelRgbFloat(data[0], data[1], data[2]);
    }
    static void write_storage(float* data, PixelRgbFloat pix) {
      data[0] = pix.r; data[1] = pix.g; data[2] = pix.b;
    }

    static PixelRgbFloat from_rgb(const RgbSpectrum& spectrum) {
      return PixelRgbFloat(spectrum.rgb.x, spectrum.rgb.y, spectrum.rgb.z);
    }
    static RgbSpectrum to_rgb(PixelRgbFloat pix) {
      return RgbSpectrum(pix.r, pix.g, pix.b);
    }
  };

  Image<PixelRgb8> read_image(FILE* input);
  void write_image_png(FILE* output, const Image<PixelRgb8>& img);
  void write_image_ppm(FILE* output, const Image<PixelRgb8>& img);
  void write_image_rgbe(FILE* output, const Image<PixelRgbFloat>& img);
}

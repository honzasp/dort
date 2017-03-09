#include "dort/image.hpp"
#include "dort/math.hpp"
#include "dort/tonemap.hpp"

namespace dort {
  Image<PixelRgb8> tonemap_srgb(const Image<PixelRgbFloat>& image, float scale) {
    return tonemap_channels(image, [&](float linear) -> uint8_t {
      linear = linear * scale;
      float gamma = linear >= 0.0031308f
        ? 1.055f * pow(linear, 1.f/2.4f) - 0.055f
        : linear * 12.92f;
      return (uint8_t)clamp((int32_t)(gamma * 256.f), 0, 255);
    });
  }

  Image<PixelRgb8> tonemap_gamma(const Image<PixelRgbFloat>& image,
      float gamma, float scale)
  {
    return tonemap_channels(image, [&](float linear) -> uint8_t {
      return (uint8_t)clamp((int32_t)pow(linear * scale, 1.f/gamma));
    });
  }

  template<class F>
  Image<PixelRgb8> tonemap_pixel(const Image<PixelRgbFloat>& image, F pixel_map) {
    Image<PixelRgb8> out_image(image.x_res, image.y_res);
    for(uint32_t y = 0; y < image.y_res; ++y) {
      for(uint32_t x = 0; x < image.x_res; ++x) {
        out_image.set_pixel(x, y, pixel_map(image.get_pixel(x, y)));
      }
    }
    return out_image;
  }

  template<class F>
  Image<PixelRgb8> tonemap_channels(const Image<PixelRgbFloat>& image, F channel_map) {
    return tonemap_pixel(image, [&](const PixelRgbFloat& pixel) {
      return PixelRgb8(channel_map(pixel.r), channel_map(pixel.g), channel_map(pixel.b));
    });
  }
}

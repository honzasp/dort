#pragma once
#include "dort/dort.hpp"

namespace dort {
  Image<PixelRgb8> tonemap_srgb(const Image<PixelRgbFloat>& image, float scale);
  Image<PixelRgb8> tonemap_gamma(const Image<PixelRgbFloat>& image,
      float gamma, float scale);

  template<class F>
  Image<PixelRgb8> tonemap_pixel(const Image<PixelRgbFloat>& image, F pixel_map);
  template<class F>
  Image<PixelRgb8> tonemap_channels(const Image<PixelRgbFloat>& image, F channel_map);

}

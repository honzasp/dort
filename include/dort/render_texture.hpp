#pragma once
#include "dort/image.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  Image<PixelRgb8> render_texture_2d(
      std::shared_ptr<Texture2d<Spectrum>> texture,
      Vec2i pixel_res, Vec2 texture_scale);
}

#include "dort/render_texture.hpp"
#include "dort/texture.hpp"

namespace dort {
  Image<PixelRgb8> render_texture_2d(
      std::shared_ptr<Texture2d<Spectrum>> texture,
      Vec2i pixel_res, Vec2 texture_scale)
  {
    Vec2 inv_texture_scale = 1.f / texture_scale;

    Image<PixelRgb8> image(pixel_res.x, pixel_res.y);
    for(int32_t y = 0; y < pixel_res.y; ++y) {
      for(int32_t x = 0; x < pixel_res.x; ++x) {
        Vec2 tex_p = Vec2(float(x), float(y)) * inv_texture_scale;
        Spectrum color = texture->evaluate(tex_p);
        image.set_rgb(x, y, color);
      }
    }

    return image;
  }
}

#include "dort/image.hpp"
#include "dort/image_texture.hpp"

namespace dort {
  Spectrum ImageTexture::evaluate(Vec2 v) const {
    float wrapped_x = v.x - floor(v.x);
    float wrapped_y = v.y - floor(v.y);
    uint32_t pixel_x = floor_int32(float(this->image->x_res) * wrapped_x);
    uint32_t pixel_y = floor_int32(float(this->image->y_res) * wrapped_y);
    return this->image->get_rgb(pixel_x, pixel_y);
  }
}

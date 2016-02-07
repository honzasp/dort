#include "dort/image.hpp"
#include "dort/image_texture.hpp"

namespace dort {
  Spectrum ImageTexture::evaluate(const DiffGeom& diff_geom) const {
    Tex2 st = this->texture_map->map(diff_geom);
    float wrapped_s = st.s - floor(st.s);
    float wrapped_t = st.t - floor(st.t);
    uint32_t pixel_x = floor_int32(float(this->image->x_res) * wrapped_s);
    uint32_t pixel_y = floor_int32(float(this->image->y_res) * wrapped_t);
    return this->image->get_rgb(pixel_x, pixel_y);
  }
}

#pragma once
#include <vector>
#include "dort/atomic_spectrum.hpp"
#include "dort/image.hpp"
#include "dort/rect_i.hpp"
#include "dort/sampled_filter.hpp"
#include "dort/vec_2.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  struct Film {
    struct Pixel {
      Spectrum color;
      float weight;
      AtomicSpectrum splat;
    };

    Vec2i res;
    std::vector<Pixel> pixels;
    SampledFilter filter;
    float splat_scale;

    Film(uint32_t x_res, uint32_t y_res, std::shared_ptr<Filter> filter);
    Film(uint32_t x_res, uint32_t y_res, SampledFilter filter);
    void add_sample(Vec2 pos, const Spectrum& radiance);
    void add_splat(Vec2 pos, const Spectrum& radiance);
    void add_tile(Vec2i pos, const Film& tile);
    template<class Pix>
    Image<Pix> to_image() const;

    uint32_t pixel_idx(int32_t x, int32_t y) const {
      assert(x >= 0 && x < int32_t(this->res.x));
      assert(y >= 0 && y < int32_t(this->res.y));
      return this->res.x * y + x;
    }

    Recti get_pixel_rect(Vec2 pos) const;
  };
}

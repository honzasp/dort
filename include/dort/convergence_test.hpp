#pragma once
#include "dort/dort.hpp"

namespace dort {
  std::string test_convergence(CtxG& ctx,
      const Image<PixelRgbFloat>& reference,
      const Image<PixelRgbFloat>& tested,
      int32_t min_tile_size, float variation, float bias, float p_value);
}

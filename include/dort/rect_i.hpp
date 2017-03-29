#pragma once
#include "dort/vec_2i.hpp"

namespace dort {
  struct Recti {
    Vec2i p_min;
    Vec2i p_max;

    Recti(): Recti(0, 0, 0, 0) { }
    Recti(const Vec2i& p_min, const Vec2i& p_max):
      p_min(p_min), p_max(p_max) { }
    Recti(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y):
      p_min(min_x, min_y), p_max(max_x, max_y) { }
    Recti(int32_t x, int32_t y):
      p_min(0, 0), p_max(x, y) { }
  };
}

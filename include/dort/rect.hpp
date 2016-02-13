#pragma once
#include "dort/vec_2.hpp"

namespace dort {
  struct Rect {
    Vec2 p_min;
    Vec2 p_max;

    Rect(): Rect(0.f, 0.f, 0.f, 0.f) { }
    Rect(const Vec2& p_min, const Vec2& p_max):
      p_min(p_min), p_max(p_max) { }
    Rect(float min_x, float min_y, float max_x, float max_y):
      p_min(min_x, min_y), p_max(max_x, max_y) { }
    Rect(float x, float y):
      p_min(0.f, 0.f), p_max(x, y) { }
  };
}

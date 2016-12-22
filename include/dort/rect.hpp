#pragma once
#include "dort/vec_2.hpp"

namespace dort {
  struct Rect {
    Vec2 p_min;
    Vec2 p_max;

    Rect(): Rect(INFINITY, INFINITY, -INFINITY, -INFINITY) { }
    Rect(const Vec2 p_min, Vec2 p_max):
      p_min(p_min), p_max(p_max) { }
    Rect(float min_x, float min_y, float max_x, float max_y):
      p_min(min_x, min_y), p_max(max_x, max_y) { }
    Rect(float x, float y):
      p_min(0.f, 0.f), p_max(x, y) { }

    bool contains(Vec2 pt) const {
      return
        this->p_min.x <= pt.x && pt.x <= this->p_max.x &&
        this->p_min.y <= pt.y && pt.y <= this->p_max.y;
    }

    Vec2 sample(Vec2 uv) const {
      return Vec2(
        lerp(uv.x, this->p_min.x, this->p_max.x),
        lerp(uv.y, this->p_min.y, this->p_max.y));
    }
  };

  Rect union_rect(const Rect& rect, Vec2 pt);
}

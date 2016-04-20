#include "dort/rect.hpp"

namespace dort {
  Rect union_rect(const Rect& rect, Vec2 pt) {
    return Rect(
        Vec2(min(rect.p_min.x, pt.x), min(rect.p_min.y, pt.y)),
        Vec2(max(rect.p_max.x, pt.x), max(rect.p_max.y, pt.y)));
  }
}


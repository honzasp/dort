#include "dort/geometry.hpp"

namespace dort {
  Box union_box(const Box& b1, const Box& b2) {
    return Box(
        Point(
          min(b1.p_min.v.x, b2.p_min.v.x),
          min(b1.p_min.v.y, b2.p_min.v.y),
          min(b1.p_min.v.z, b2.p_min.v.z)),
        Point(
          max(b1.p_max.v.x, b2.p_max.v.x),
          max(b1.p_max.v.y, b2.p_max.v.y),
          max(b1.p_max.v.z, b2.p_max.v.z)));
  }
}

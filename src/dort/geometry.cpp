#include "dort/geometry.hpp"

namespace dort {
  void coordinate_system(const Vector& vec0, Vector& vec1, Vector& vec2) {
    if(abs(vec0.v.x) > abs(vec0.v.y)) {
      vec1 = normalize(Vector(-vec0.v.z, 0.f, vec0.v.x));
    } else {
      vec1 = normalize(Vector(0.f, -vec0.v.z, vec0.v.y));
    }
    vec2 = cross(vec0, vec1);
  }
}

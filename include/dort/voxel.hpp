#pragma once
#include "dort/dort.hpp"

namespace dort {
  using Voxel = int16_t;
  constexpr Voxel VOXEL_EMPTY = 0;
  constexpr Voxel VOXEL_WILDCARD = (1 << 12) - 1;

  inline bool unify_voxels(Voxel a, Voxel b, Voxel& out) {
    if(a == b) {
      out = a;
      return true;
    } else if(b == VOXEL_WILDCARD) {
      out = a;
      return true;
    } else if(a == VOXEL_WILDCARD) {
      out = b;
      return true;
    } else {
      return false;
    }
  }
}

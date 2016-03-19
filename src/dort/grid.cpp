#include "dort/grid.hpp"

namespace dort {
  Grid::Grid() { }

  int16_t Grid::get(const Vec3i& pos) const {
    Vec3i lump_pos = Grid::lump_pos(pos);
    auto iter = this->lumps.find(lump_pos);
    if(iter == this->lumps.end()) {
      return 0;
    } else {
      return iter->second.get(pos - Lump::RADIUS * lump_pos);
    }
  }

  void Grid::set(const Vec3i& pos, int16_t item) {
    Vec3i lump_pos = Grid::lump_pos(pos);
    auto iter = this->lumps.find(lump_pos);
    if(iter == this->lumps.end()) {
      iter = this->lumps.insert(std::make_pair(lump_pos, Lump())).first;
    }
    iter->second.set(pos - Lump::RADIUS * lump_pos, item);
  }

  Grid::Finger Grid::finger(const Vec3i& pos) const {
    Finger finger;
    finger.lump_pos = Grid::lump_pos(pos);
    finger.pos = pos;

    auto iter = this->lumps.find(finger.lump_pos);
    if(iter == this->lumps.end()) {
      finger.lump = 0;
    } else {
      finger.lump = &iter->second;
    }

    return finger;
  }

  Grid::Finger Grid::shift_finger(
      const Finger& finger, const Vec3i& shift) const 
  {
    Finger next_finger;
    next_finger.pos = finger.pos + shift;
    next_finger.lump_pos = Grid::lump_pos(next_finger.pos);
    if(next_finger.lump_pos == finger.lump_pos) {
      next_finger.lump = finger.lump;
    } else {
      auto iter = this->lumps.find(next_finger.lump_pos);
      if(iter == this->lumps.end()) {
        next_finger.lump = 0;
      } else {
        next_finger.lump = &iter->second;
      }
    }
    return next_finger;
  }

  Grid::Finger Grid::shift_finger_by_one(
      const Finger& finger, uint8_t axis, bool negative) const
  {
    Vec3i shift;
    shift[axis] = negative ? -1 : 1;
    return this->shift_finger(finger, shift);
  }
}

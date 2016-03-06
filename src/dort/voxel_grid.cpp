#include "dort/voxel_grid.hpp"

namespace dort {
  VoxelGrid::VoxelGrid() {
  }

  Voxel VoxelGrid::voxel(const Vec3i& pos) const {
    Vec3i lump_pos = VoxelGrid::lump_pos(pos);
    auto iter = this->lumps.find(lump_pos);
    if(iter == this->lumps.end()) {
      return VOXEL_EMPTY;
    } else {
      return iter->second.voxel(pos - VoxelLump::RADIUS * lump_pos);
    }
  }

  void VoxelGrid::set_voxel(const Vec3i& pos, Voxel voxel) {
    Vec3i lump_pos = VoxelGrid::lump_pos(pos);
    auto iter = this->lumps.find(lump_pos);
    if(iter == this->lumps.end()) {
      iter = this->lumps.insert(std::make_pair(lump_pos, VoxelLump())).first;
    }
    iter->second.set_voxel(pos - VoxelLump::RADIUS * lump_pos, voxel);
  }

  void VoxelGrid::set_lump(const Vec3i& lump_pos, VoxelLump lump) {
    auto iter = this->lumps.find(lump_pos);
    if(iter == this->lumps.end()) {
      this->lumps.insert(std::make_pair(lump_pos, std::move(lump)));
    } else {
      iter->second = std::move(lump);
    }
  }

  VoxelGrid::Finger VoxelGrid::finger(const Vec3i& pos) const {
    Finger finger;
    finger.lump_pos = VoxelGrid::lump_pos(pos);
    finger.pos = pos;

    auto iter = this->lumps.find(finger.lump_pos);
    if(iter == this->lumps.end()) {
      finger.lump = 0;
    } else {
      finger.lump = &iter->second;
    }

    return finger;
  }

  VoxelGrid::Finger VoxelGrid::shift_finger(
      const Finger& finger, const Vec3i& shift) const 
  {
    Finger next_finger;
    next_finger.pos = finger.pos + shift;
    next_finger.lump_pos = VoxelGrid::lump_pos(next_finger.pos);
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

  VoxelGrid::Finger VoxelGrid::shift_finger_by_one(
      const Finger& finger, uint8_t axis, bool negative) const
  {
    Vec3i shift;
    shift[axis] = negative ? -1 : 1;
    return this->shift_finger(finger, shift);
  }

  bool VoxelGrid::homogeneous(const Boxi& box, Voxel& out_voxel) const {
    Voxel voxel = this->voxel(box.p_min);
    for(int32_t z = box.p_min.z; z < box.p_max.z; ++z) {
      for(int32_t y = box.p_min.y; y < box.p_max.y; ++y) {
        for(int32_t x = box.p_min.x; x < box.p_max.x; ++x) {
          if(!unify_voxels(voxel, this->voxel(Vec3i(x, y, z)), voxel)) {
            return false;
          }
        }
      }
    }

    out_voxel = voxel;
    return true;
  }
}

#pragma once
#include <unordered_map>
#include <vector>
#include "dort/box_i.hpp"
#include "dort/voxel.hpp"

namespace dort {
  struct VoxelLump {
    static constexpr int32_t RADIUS = 16;
    static constexpr int32_t SIZE = RADIUS * RADIUS * RADIUS;
    std::vector<Voxel> voxels;

    VoxelLump(): voxels(SIZE, VOXEL_EMPTY) { }

    Voxel voxel(const Vec3i& pos) const {
      return this->voxels.at(this->index(pos));
    }

    void set_voxel(const Vec3i& pos, Voxel voxel) {
      this->voxels.at(this->index(pos)) = voxel;
    }

    uint32_t index(const Vec3i& pos) const {
      assert(pos.x >= 0 && pos.x < RADIUS);
      assert(pos.y >= 0 && pos.y < RADIUS);
      assert(pos.z >= 0 && pos.z < RADIUS);
      return pos.x + pos.y * RADIUS + pos.z * RADIUS * RADIUS;
    }
  };

  class VoxelGrid {
    std::unordered_map<Vec3i, VoxelLump> lumps;
  public:
    VoxelGrid();

    Voxel voxel(const Vec3i& pos) const;
    void set_voxel(const Vec3i& pos, Voxel voxel);
    void set_lump(const Vec3i& lump_pos, VoxelLump lump);

    static Vec3i lump_pos(const Vec3i& pos) {
      uint32_t r = VoxelLump::RADIUS;
      return Vec3i(
          pos.x >= 0 ? pos.x / r : -(-pos.x / r),
          pos.y >= 0 ? pos.y / r : -(-pos.y / r),
          pos.z >= 0 ? pos.z / r : -(-pos.z / r));
    }

    static Vec3i voxel_in_lump_pos(const Vec3i& pos) {
      return pos - VoxelLump::RADIUS * lump_pos(pos);
    }

    class Finger {
      friend class VoxelGrid;

      const VoxelLump* lump;
      Vec3i lump_pos;
      Vec3i pos;
    public:
      Voxel voxel() const {
        if(this->lump == 0) {
          return VOXEL_EMPTY;
        } else {
          return this->lump->voxel(VoxelGrid::voxel_in_lump_pos(this->pos));
        }
      }

      Vec3i position() const {
        return this->pos;
      }
    };

    Finger finger(const Vec3i& pos) const;
    Finger shift_finger(const Finger& finger, const Vec3i& shift) const;
    Finger shift_finger_by_one(const Finger& finger, uint8_t axis, bool negative) const;

    bool homogeneous(const Boxi& box, Voxel& out_voxel) const;
  };
}

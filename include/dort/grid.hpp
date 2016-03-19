#pragma once
#include <unordered_map>
#include <vector>
#include "dort/box_i.hpp"

namespace dort {
  class Grid {
    struct Lump {
      static constexpr int32_t RADIUS = 16;
      static constexpr int32_t SIZE = RADIUS * RADIUS * RADIUS;
      std::vector<int16_t> items;

      Lump(): items(SIZE, 0) { }

      int16_t get(const Vec3i& pos) const {
        return this->items.at(this->index(pos));
      }

      void set(const Vec3i& pos, int16_t item) {
        this->items.at(this->index(pos)) = item;
      }

      uint32_t index(const Vec3i& pos) const {
        assert(pos.x >= 0 && pos.x < RADIUS);
        assert(pos.y >= 0 && pos.y < RADIUS);
        assert(pos.z >= 0 && pos.z < RADIUS);
        return pos.x + pos.y * RADIUS + pos.z * RADIUS * RADIUS;
      }
    };

    std::unordered_map<Vec3i, Lump> lumps;
  public:
    Grid();

    int16_t get(const Vec3i& pos) const;
    void set(const Vec3i& pos, int16_t item);

    static Vec3i lump_pos(const Vec3i& pos) {
      uint32_t r = Lump::RADIUS;
      return Vec3i(floor_div(pos.x, r), floor_div(pos.y, r), floor_div(pos.z, r));
    }

    class Finger {
      friend class Grid;

      const Lump* lump;
      Vec3i lump_pos;
      Vec3i pos;
    public:
      int16_t get() const {
        if(this->lump == 0) {
          return 0;
        }
        return this->lump->get(this->pos - Lump::RADIUS * this->lump_pos);
      }

      Vec3i position() const {
        return this->pos;
      }
    };

    Finger finger(const Vec3i& pos) const;
    Finger shift_finger(const Finger& finger, const Vec3i& shift) const;
    Finger shift_finger_by_one(const Finger& finger, uint8_t axis, bool negative) const;
  };
}

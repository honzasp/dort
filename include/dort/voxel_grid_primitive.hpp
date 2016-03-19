#pragma once
#include <vector>
#include "dort/box_i.hpp"
#include "dort/primitive.hpp"
#include "dort/voxel.hpp"

namespace dort {
  class VoxelGridPrimitive final: public GeometricPrimitive {
    enum class NodeType: uint8_t {
      LEAF_LEAF = 0,
      LEAF_BRANCH = 1,
      SHORT_BRANCH_BRANCH = 2,
      LONG_BRANCH_BRANCH = 3,
    };

    struct Node {
      uint32_t bits;
      Node(): bits(-1u) { }
      Node(uint32_t bits): bits(bits) { }

      uint32_t field(uint32_t offset, uint32_t length) const {
        return (this->bits >> offset) & ((1 << length) - 1);
      }

      uint8_t axis() const { return this->field(0, 2); }
      NodeType type() const { return NodeType(this->field(2, 2)); }
      bool full_bit_1() const { return this->field(4, 1) != 0; }
      bool full_bit_2() const { return this->field(5, 1) != 0; }
      bool leaf_branch_is_leaf_left() const { return this->field(6, 1) != 0; }
      Voxel voxel_1() const { return this->field(18, 14); }
      Voxel voxel_2() const { return this->field(4, 14); }
      uint32_t short_branch_offset() const { return this->field(6, 26); }
      uint32_t long_branch_offset() const { return this->field(4, 28); }

      static constexpr uint32_t VOXEL_MASK = (1 << 14) - 1;

      static Node make_leaf_leaf(uint8_t axis, Voxel left, Voxel right) {
        return Node(axis | (uint32_t(NodeType::LEAF_LEAF) << 2) 
            | ((right & VOXEL_MASK) << 4) | ((left & VOXEL_MASK) << 18));
      }
      static Node make_leaf_branch(uint8_t axis, Voxel leaf,
          bool is_leaf_left, bool full_bit_branch) 
      {
        return Node(axis | (uint32_t(NodeType::LEAF_BRANCH) << 2)
            | (full_bit_branch ? 1 << 4 : 0)
            | (is_leaf_left ? 1 << 6 : 0) | ((leaf & VOXEL_MASK) << 18));
      }
      static Node make_branch_branch(uint8_t axis, uint32_t branch_offset, 
          bool full_bit_left, bool full_bit_right)
      {
        if(branch_offset < (1 << 26)) {
          return Node(axis | (uint32_t(NodeType::SHORT_BRANCH_BRANCH) << 2)
              | (full_bit_left ? 1 << 4 : 0) | (full_bit_right ? 1 << 5 : 0)
              | (branch_offset << 6));
        } else {
          return Node(axis | (uint32_t(NodeType::LONG_BRANCH_BRANCH) << 2)
              | (branch_offset << 4));
        }
      }
    };

    struct BranchOrLeaf {
      bool is_leaf;
      union {
        Voxel leaf_voxel;
        struct {
          uint32_t branch_idx;
          bool branch_full;
        };
      };
    };

    Boxi root_box;
    std::vector<Node> nodes;
    Transform voxel_to_frame;
  public:
    VoxelGridPrimitive(const Boxi& grid_box, const Grid& grid,
        const Transform& voxel_to_frame);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  private:
    BranchOrLeaf build_node(const Grid& grid, const Boxi& box);

    struct VoxelRay {
      Vec3 orig;
      Vec3 dir_inv;
      Vec3 dir;
      float t_max;
      float t_min;
      uint8_t dir_is_neg[3];
    };

    struct RayEntry {
      Vec3 p_hit;
      float t_hit;
      bool on_surface;
      uint8_t surface_axis;
      bool surface_neg;
    };

    template<class F>
    bool traverse(const Ray& ray, F callback) const;
    template<class F>
    bool traverse_walk(const VoxelRay& ray, uint32_t node_idx,
        const Boxi& box, const RayEntry& entry, const RayEntry& exit, F callback) const;
    template<class F>
    bool traverse_walk_single_half(const VoxelRay& ray, uint32_t node_idx,
        const Node& node, const Boxi& half_box, bool left_half,
        const RayEntry& half_entry, const RayEntry& half_exit,
        F callback) const;
    template<class F>
    bool traverse_walk_both_halves(const VoxelRay& ray,
        uint32_t node_idx, const Node& node, uint8_t axis,
        const Boxi& left_box, const Boxi& right_box,
        const RayEntry& entry, const RayEntry& exit, const RayEntry& mid_entry,
        F callback) const;

    static bool ray_box_hit(const VoxelRay& ray, const Boxi& box,
        RayEntry& out_entry, RayEntry& out_exit);
    static bool is_box_homogeneous(const Grid& grid,
        const Boxi& box, Voxel& out_voxel);
  };
}

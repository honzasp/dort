#include "dort/bsdf.hpp"
#include "dort/grid.hpp"
#include "dort/material.hpp"
#include "dort/voxel_grid_primitive.hpp"

namespace dort {
  VoxelGridPrimitive::VoxelGridPrimitive(
      const Boxi& grid_box, const Grid& grid,
      const Transform& voxel_to_frame,
      VoxelMaterials voxel_materials):
    root_box(grid_box),
    voxel_to_frame(voxel_to_frame),
    voxel_materials(std::move(voxel_materials))
  {
    auto ret = this->build_node(grid, this->root_box);
    if(ret.is_leaf) {
      assert(this->nodes.empty());
      this->nodes.push_back(Node::make_leaf_leaf(0, ret.leaf_voxel, ret.leaf_voxel));
    } else {
      assert(!this->nodes.empty());
    }
  }

  bool VoxelGridPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    bool miss = this->traverse(ray, 
        [&](const RayEntry& entry, const RayEntry& exit, Voxel voxel) 
    {
      assert(voxel != VOXEL_EMPTY);
      assert(entry.t_hit >= ray.t_min && entry.t_hit <= ray.t_max);

      if(voxel < 0) {
        Vector voxel_ray_dir = this->voxel_to_frame.apply_inv(ray.dir);
        Vec3 entry_pos = floor(entry.p_hit + 1e-3f * voxel_ray_dir.v);

        float pad = entry.on_surface ? 1e-3f : 0.f;
        Ray prim_ray(Point(entry.p_hit - entry_pos), voxel_ray_dir,
          -pad, exit.t_hit - entry.t_hit + pad);
        const auto& prim = this->voxel_materials.prim_voxels.at(-voxel);
        if(!prim->intersect(prim_ray, out_isect)) {
          return true;
        }

        out_isect.world_diff_geom = this->voxel_to_frame.apply(
          translate(Vector(entry_pos)).apply(out_isect.world_diff_geom));
        ray.t_max = min(ray.t_max, prim_ray.t_max + entry.t_hit);
        return false;
      } else {
        DiffGeom voxel_geom;
        voxel_geom.p = Point(entry.p_hit);
        out_isect.aux_int32[0] = voxel;
        if(entry.on_surface) {
          out_isect.aux_int32[1] = entry.surface_axis + (entry.surface_neg ? 3 : 0);
        } else {
          out_isect.aux_int32[1] = 6;
        }

        if(entry.on_surface) {
          voxel_geom.nn.v[entry.surface_axis] = entry.surface_neg ? 1.f : -1.f;
          // TODO
          voxel_geom.u = 0.5f;
          voxel_geom.v = 0.5f;
          voxel_geom.dpdu = Vector();
          voxel_geom.dpdv = Vector();
        } else {
          voxel_geom.nn = normalize(Normal(-ray.dir));
          voxel_geom.u = 0.5f;
          voxel_geom.v = 0.5f;
          voxel_geom.dpdu = Vector();
          voxel_geom.dpdv = Vector();
        }
          
        out_isect.frame_diff_geom = this->voxel_to_frame.apply(voxel_geom);
        out_isect.world_diff_geom = out_isect.frame_diff_geom;
        out_isect.ray_epsilon = 1e-3;
        out_isect.primitive = this;

        ray.t_max = entry.t_hit;
        return false;
      }
    });

    return !miss;
  }

  bool VoxelGridPrimitive::intersect_p(const Ray& ray) const {
    bool miss = this->traverse(ray, 
        [&](const RayEntry& entry, const RayEntry& exit, Voxel voxel) 
    {
      assert(voxel != VOXEL_EMPTY);
      if(voxel < 0) {
        Vec3 orig(entry.p_hit - floor(entry.p_hit));
        if(entry.on_surface) {
          orig[entry.surface_axis] = entry.surface_neg ? 1.f : 0.f;
        }

        Ray prim_ray(Point(orig), this->voxel_to_frame.apply_inv(ray.dir),
          1e-2, exit.t_hit - entry.t_hit);
        const auto& prim = this->voxel_materials.prim_voxels.at(-voxel);

        return !prim->intersect_p(prim_ray);
      } else {
        return false;
      }
    });
    return !miss;
  }

  Box VoxelGridPrimitive::bounds() const {
    Box box(Point(Vec3(this->root_box.p_min)), Point(Vec3(this->root_box.p_max)));
    return this->voxel_to_frame.apply(box);
  }

  std::unique_ptr<Bsdf> VoxelGridPrimitive::get_bsdf(const Intersection& isect) const {
    Voxel voxel = isect.aux_int32[0];
    uint32_t face = isect.aux_uint32[1];
    assert(voxel != VOXEL_EMPTY);
    assert(voxel > 0);
    assert(face <= 6);

    if(face == 6) {
      return std::make_unique<Bsdf>(isect.world_diff_geom, isect.frame_diff_geom.nn);
    }
    const auto& cube_material = this->voxel_materials.cube_voxels.at(voxel);
    return cube_material.faces.at(face)->get_bsdf(isect.frame_diff_geom);
  }

  const Light* VoxelGridPrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }

  VoxelGridPrimitive::BranchOrLeaf VoxelGridPrimitive::build_node(
      const Grid& grid, const Boxi& box) 
  {
    BranchOrLeaf ret;
    if(VoxelGridPrimitive::is_box_homogeneous(grid, box, ret.leaf_voxel)) {
      Vec3i r = box.p_max - box.p_min;
      if(ret.leaf_voxel >= 0 || (r.x <= 1 && r.y <= 1 && r.z <= 1)) {
        ret.is_leaf = true;
        return ret;
      }
    }

    // TODO: compute best axis for small boxes
    uint8_t axis = box.max_axis();
    auto left_right_box = box.split(axis);
    Boxi left_box = std::get<0>(left_right_box);
    Boxi right_box = std::get<1>(left_right_box);

    uint32_t idx = this->nodes.size();
    this->nodes.emplace_back();

    BranchOrLeaf left = this->build_node(grid, left_box);
    BranchOrLeaf right = this->build_node(grid, right_box);

    bool full;
    if(left.is_leaf && right.is_leaf) {
      this->nodes.at(idx) = Node::make_leaf_leaf(axis,
          left.leaf_voxel, right.leaf_voxel);
      full = left.leaf_voxel > 0 && right.leaf_voxel > 0;
    } else if(left.is_leaf && !right.is_leaf) {
      assert(right.branch_idx == idx + 1);
      this->nodes.at(idx) = Node::make_leaf_branch(axis, left.leaf_voxel,
          true, right.branch_full);
      full = left.leaf_voxel > 0 && right.branch_full;
    } else if(!left.is_leaf && right.is_leaf) {
      assert(left.branch_idx == idx + 1);
      this->nodes.at(idx) = Node::make_leaf_branch(axis, right.leaf_voxel,
          false, left.branch_full);
      full = right.leaf_voxel > 0 && left.branch_full;
    } else {
      assert(left.branch_idx == idx + 1);
      assert(right.branch_idx > idx);
      this->nodes.at(idx) = Node::make_branch_branch(axis,
          right.branch_idx - idx, left.branch_full, right.branch_full);
      full = left.branch_full && right.branch_full;
    }

    ret.is_leaf = false;
    ret.branch_idx = idx;
    ret.branch_full = full;
    return ret;
  }

  template<class F>
  bool VoxelGridPrimitive::traverse(const Ray& ray, F callback) const {
    VoxelRay voxel_ray;
    voxel_ray.orig = this->voxel_to_frame.apply_inv(ray.orig).v;
    voxel_ray.dir = this->voxel_to_frame.apply_inv(ray.dir).v;
    voxel_ray.dir_inv = 1.f / voxel_ray.dir;
    voxel_ray.t_max = ray.t_max;
    voxel_ray.t_min = ray.t_min;

    voxel_ray.dir_is_neg[0] = voxel_ray.dir[0] < 0.f;
    voxel_ray.dir_is_neg[1] = voxel_ray.dir[1] < 0.f;
    voxel_ray.dir_is_neg[2] = voxel_ray.dir[2] < 0.f;

    RayEntry entry;
    RayEntry exit;
    if(!VoxelGridPrimitive::ray_box_hit(voxel_ray, this->root_box, entry, exit)) {
      return true;
    }

    return this->traverse_walk(voxel_ray, 0, this->root_box, entry, exit, callback);
  }

  template<class F>
  bool VoxelGridPrimitive::traverse_walk(const VoxelRay& ray, uint32_t node_idx,
      const Boxi& box, const RayEntry& entry, const RayEntry& exit, F callback) const
  {
    assert(entry.t_hit >= ray.t_min && entry.t_hit <= ray.t_max);
    assert(exit.t_hit >= ray.t_min && exit.t_hit <= ray.t_max);
    assert(exit.t_hit - entry.t_hit > -1e-3f);

    const Node& node = this->nodes.at(node_idx);
    uint8_t axis = node.axis();

    auto box_left_right = box.split(axis);
    Boxi left_box = std::get<0>(box_left_right);
    Boxi right_box = std::get<1>(box_left_right);
    int32_t mid = std::get<2>(box_left_right);
    float mid_f = float(mid);

    float t_mid = (mid_f - ray.orig[axis]) * ray.dir_inv[axis];

    RayEntry mid_entry;
    if(t_mid < ray.t_min) {
      mid_entry.t_hit = ray.t_min;
      mid_entry.on_surface = false;
    } else if(t_mid > ray.t_max) {
      mid_entry.t_hit = ray.t_max;
      mid_entry.on_surface = false;
    } else {
      mid_entry.t_hit = t_mid;
      mid_entry.on_surface = true;
      mid_entry.surface_axis = axis;
      mid_entry.surface_neg = ray.dir_is_neg[axis];
    }
    mid_entry.p_hit = ray.orig + mid_entry.t_hit * ray.dir;

    bool only_right = false;
    bool only_left = false;
    if(t_mid <= ray.t_min) {
      if(ray.dir_is_neg[axis]) {
        only_left = true;
      } else {
        only_right = true;
      }
    } else if(t_mid >= ray.t_max) {
      if(ray.dir_is_neg[axis]) {
        only_right = true;
      } else {
        only_left = true;
      }
    } else if(mid_f <= entry.p_hit[axis] && mid_f <= exit.p_hit[axis]) {
      only_right = true;
    } else if(mid_f >= entry.p_hit[axis] && mid_f >= exit.p_hit[axis]) {
      only_left = true;
    }

    if(only_right || only_left) {
      return this->traverse_walk_single_half(ray, node_idx, node,
          (only_left ? left_box : right_box), only_left,
          entry, exit, callback);
    }

    assert(mid_entry.on_surface);
    assert(mid_entry.t_hit - entry.t_hit > -1e-3f);
    assert(mid_entry.t_hit - exit.t_hit < 1e-3f);
    //assert(box.contains(floor_vec3i(mid_entry.p_hit)));
    return this->traverse_walk_both_halves(ray, node_idx, node, axis,
        left_box, right_box, entry, exit, mid_entry, callback);
  }

  template<class F>
  bool VoxelGridPrimitive::traverse_walk_single_half(const VoxelRay& ray,
      uint32_t node_idx, const Node& node,
      const Boxi& half_box, bool left_half,
      const RayEntry& half_entry, const RayEntry& half_exit,
      F callback) const
  {
    NodeType type = node.type();
    if(type == NodeType::LEAF_LEAF) {
      Voxel voxel = left_half ? node.voxel_1() : node.voxel_2();
      if(voxel == VOXEL_EMPTY) {
        return true;
      }
      return callback(half_entry, half_exit, voxel);
    } else if(type == NodeType::LEAF_BRANCH) {
      bool leaf_left = node.leaf_branch_is_leaf_left();
      bool walk_branch = (leaf_left && !left_half) || (!leaf_left && left_half);
      if(walk_branch) {
        return this->traverse_walk(ray, node_idx + 1,
          half_box, half_entry, half_exit, callback);
      } else {
        Voxel voxel = node.voxel_1();
        if(voxel == VOXEL_EMPTY) {
          return true;
        }
        return callback(half_entry, half_exit, voxel);
      }
    } else {
      uint32_t offset = left_half ? 1 :
        (type == NodeType::SHORT_BRANCH_BRANCH 
          ? node.short_branch_offset() : node.long_branch_offset());
      return this->traverse_walk(ray, node_idx + offset,
          half_box, half_entry, half_exit, callback);
    }
  }

  template<class F>
  bool VoxelGridPrimitive::traverse_walk_both_halves(const VoxelRay& ray,
      uint32_t node_idx, const Node& node, uint8_t axis,
      const Boxi& left_box, const Boxi& right_box,
      const RayEntry& entry, const RayEntry& exit, const RayEntry& mid_entry,
      F callback) const
  {
    assert(entry.t_hit >= ray.t_min && entry.t_hit <= ray.t_max);
    assert(exit.t_hit >= ray.t_min && exit.t_hit <= ray.t_max);
    assert(mid_entry.t_hit >= ray.t_min && mid_entry.t_hit <= ray.t_max);

    NodeType type = node.type();
    bool right_first = ray.dir_is_neg[axis];
    bool go_on = true;

    if(type == NodeType::LEAF_LEAF) {
      Voxel left = node.voxel_1();
      Voxel right = node.voxel_2();

      if(right_first) {
        if(right != VOXEL_EMPTY) {
          go_on = callback(entry, mid_entry, right);
        }
        if(go_on && left != VOXEL_EMPTY) {
          return callback(mid_entry, exit, left);
        }
      } else {
        if(left != VOXEL_EMPTY) {
          go_on = callback(entry, mid_entry, left);
        }
        if(go_on && right != VOXEL_EMPTY) {
          return callback(mid_entry, exit, right);
        }
      }
    } else if(type == NodeType::LEAF_BRANCH) {
      bool leaf_left = node.leaf_branch_is_leaf_left();
      Voxel leaf = node.voxel_1();
      uint32_t branch = node_idx + 1;

      if(right_first) {
        if(leaf_left) {
          go_on = this->traverse_walk(ray, branch,
                right_box, entry, mid_entry, callback);
          if(go_on && leaf != VOXEL_EMPTY) {
            return callback(mid_entry, exit, leaf);
          }
        } else {
          if(leaf != VOXEL_EMPTY) {
            go_on = callback(entry, mid_entry, leaf);
          }
          if(go_on) {
            return this->traverse_walk(ray, branch,
                left_box, mid_entry, exit, callback);
          }
        }
      } else {
        if(leaf_left) {
          if(leaf != VOXEL_EMPTY) {
            go_on = callback(entry, mid_entry, leaf);
          }
          if(go_on) {
            return this->traverse_walk(ray, branch,
                right_box, mid_entry, exit, callback);
          }
        } else {
          go_on = this->traverse_walk(ray, branch,
              left_box, entry, mid_entry, callback);
          if(go_on && leaf != VOXEL_EMPTY) {
            return callback(mid_entry, exit, leaf);
          }
        }
      }
    } else {
      uint32_t left = node_idx + 1;
      uint32_t right = node_idx + (type == NodeType::SHORT_BRANCH_BRANCH 
        ? node.short_branch_offset() : node.long_branch_offset());

      if(right_first) {
        go_on = this->traverse_walk(ray, right,
            right_box, entry, mid_entry, callback);
        if(go_on) {
          return this->traverse_walk(ray, left,
              left_box, mid_entry, exit, callback);
        }
      } else {
        go_on = this->traverse_walk(ray, left,
            left_box, entry, mid_entry, callback);
        if(go_on) {
          return this->traverse_walk(ray, right,
              right_box, mid_entry, exit, callback);
        }
      }
    }

    return go_on;
  }

  bool VoxelGridPrimitive::ray_box_hit(const VoxelRay& ray, const Boxi& box,
      RayEntry& out_entry, RayEntry& out_exit) 
  {
    auto f_min = [](float x) { return x < INFINITY ? x : -INFINITY; };
    auto f_max = [](float x) { return x > -INFINITY ? x : INFINITY; };

    float t0_entry = f_min((float(box[  ray.dir_is_neg[0]][0]) 
          - ray.orig[0]) * ray.dir_inv[0]);
    float t0_exit  = f_max((float(box[1-ray.dir_is_neg[0]][0]) 
          - ray.orig[0]) * ray.dir_inv[0]);

    float t1_entry = f_min((float(box[  ray.dir_is_neg[1]][1]) 
          - ray.orig[1]) * ray.dir_inv[1]);
    float t1_exit  = f_max((float(box[1-ray.dir_is_neg[1]][1]) 
          - ray.orig[1]) * ray.dir_inv[1]);

    float t2_entry = f_min((float(box[  ray.dir_is_neg[2]][2]) 
        - ray.orig[2]) * ray.dir_inv[2]);
    float t2_exit  = f_max((float(box[1-ray.dir_is_neg[2]][2]) 
        - ray.orig[2]) * ray.dir_inv[2]);

    if(t0_entry > t1_entry) {
      if(t0_entry > t2_entry) {
        out_entry.t_hit = t0_entry;
        out_entry.surface_axis = 0;
      } else {
        out_entry.t_hit = t2_entry;
        out_entry.surface_axis = 2;
      }
    } else {
      if(t2_entry > t1_entry) {
        out_entry.t_hit = t2_entry;
        out_entry.surface_axis = 2;
      } else {
        out_entry.t_hit = t1_entry;
        out_entry.surface_axis = 1;
      }
    }

    if(t0_exit < t1_exit) {
      if(t0_exit < t2_exit) {
        out_exit.t_hit = t0_exit;
        out_exit.surface_axis = 0;
      } else {
        out_exit.t_hit = t2_exit;
        out_exit.surface_axis = 2;
      }
    } else {
      if(t2_exit < t1_exit) {
        out_exit.t_hit = t2_exit;
        out_exit.surface_axis = 2;
      } else {
        out_exit.t_hit = t1_exit;
        out_exit.surface_axis = 1;
      }
    }

    assert(is_finite(out_entry.t_hit));
    assert(is_finite(out_exit.t_hit));

    if(out_entry.t_hit > out_exit.t_hit) {
      return false;
    } else if(out_exit.t_hit < ray.t_min) {
      return false;
    }

    if(out_entry.t_hit < ray.t_min) {
      out_entry.t_hit = ray.t_min;
      out_entry.on_surface = false;
    } else if(out_entry.t_hit > ray.t_max) {
      return false;
    } else {
      out_entry.on_surface = true;
      out_entry.surface_neg = ray.dir_is_neg[out_entry.surface_axis];
    }

    if(out_exit.t_hit > ray.t_max) {
      out_exit.t_hit = ray.t_max;
      out_exit.on_surface = false;
    } else {
      out_exit.on_surface = true;
      out_exit.surface_neg = ray.dir_is_neg[out_exit.surface_axis];
    }

    if(out_entry.t_hit > out_exit.t_hit) {
      return false;
    }

    out_entry.p_hit = ray.orig + ray.dir * out_entry.t_hit;
    out_exit.p_hit = ray.orig + ray.dir * out_exit.t_hit;
    return true;
  }

  bool VoxelGridPrimitive::is_box_homogeneous(const Grid& grid,
      const Boxi& box, Voxel& out_voxel)
  {
    Voxel voxel = grid.get(box.p_min);
    for(int32_t z = box.p_min.z; z < box.p_max.z; ++z) {
      for(int32_t y = box.p_min.y; y < box.p_max.y; ++y) {
        for(int32_t x = box.p_min.x; x < box.p_max.x; ++x) {
          if(!unify_voxels(voxel, grid.get(Vec3i(x, y, z)), voxel)) {
            return false;
          }
        }
      }
    }
    out_voxel = voxel;
    return true;
  }
}

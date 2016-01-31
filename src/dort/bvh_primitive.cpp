#include <algorithm>
#include "dort/bvh_primitive.hpp"

namespace dort {
  BvhPrimitive::BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
      uint32_t max_leaf_size, BvhSplitMethod split_method)
  {
    std::vector<PrimitiveInfo> build_infos(prims.size());
    for(uint32_t i = 0; i < prims.size(); ++i) {
      Box bounds = prims.at(i)->bounds();
      Point centroid = (bounds.p_min + bounds.p_max) * 0.5f;
      assert(is_finite(bounds));
      build_infos.at(i) = PrimitiveInfo { i, bounds, centroid };
    }

    max_leaf_size = min(max_leaf_size, 0xffu);

    // TODO: use an arena for build node allocation?
    uint32_t node_count = 0;
    auto root_node = this->build_node(build_infos, prims,
        0, prims.size(), max_leaf_size, split_method, node_count);
    this->linear_nodes.reserve(node_count);
    // TODO: assure that the linear nodes are aligned
    this->linearize_node(*root_node, 0);
  }

  bool BvhPrimitive::intersect(Ray& ray, Intersection& out_isect) const
  {
    bool found = false;
    Vector inv_dir(1.f / ray.dir.v.x, 1.f / ray.dir.v.y, 1.f / ray.dir.v.z);
    bool dir_is_neg[] = { ray.dir.v.x < 0.f, ray.dir.v.y < 0.f, ray.dir.v.z < 0.f };

    uint32_t todo_stack[this->max_depth + 1];
    uint32_t todo_top = 0;
    uint32_t todo_index = 0;
    for(;;) {
      const LinearNode& linear_node = this->linear_nodes.at(todo_index);
      if(fast_box_hit_p(linear_node.bounds, ray, inv_dir, dir_is_neg)) {
        if(linear_node.prim_count == 0) {
          if(dir_is_neg[linear_node.axis]) {
            todo_stack[todo_top++] = todo_index + 1;
            todo_index = linear_node.offset;
          } else {
            todo_stack[todo_top++] = linear_node.offset;
            todo_index = todo_index + 1;
          }
          continue;
        }

        for(uint32_t i = 0; i < linear_node.prim_count; ++i) {
          auto& prim = this->ordered_prims.at(linear_node.offset + i);
          if(prim->intersect(ray, out_isect)) {
            found = true;
          }
        }
      }

      if(todo_top == 0) {
        break;
      }
      todo_index = todo_stack[--todo_top];
    }

    return found;
  }

  Box BvhPrimitive::bounds() const 
  {
    if(this->linear_nodes.empty()) {
      return Box();
    }
    return this->linear_nodes.at(0).bounds;
  }

  Spectrum BvhPrimitive::get_color(const DiffGeom&) const 
  {
    assert("BvhPrimitive::get_color called");
    return Spectrum();
  }

  std::unique_ptr<BvhPrimitive::BuildNode> BvhPrimitive::build_node(
      std::vector<PrimitiveInfo>& build_infos,
      std::vector<std::unique_ptr<Primitive>>& prims,
      uint32_t begin, uint32_t end,
      uint32_t max_leaf_size, BvhSplitMethod split_method,
      uint32_t& out_node_count)
  {
    out_node_count += 1;

    Box bounds;
    for(uint32_t i = begin; i < end; ++i) {
      bounds = union_box(bounds, build_infos.at(i).bounds);
    }

    uint8_t axis = box_max_axis(bounds);
    float extent = (bounds.p_max - bounds.p_min).v[axis];
    uint32_t prims_begin = this->ordered_prims.size();

    if(extent == 0.f || (end - begin) <= max_leaf_size) {
      // make a leaf
      for(uint32_t i = begin; i < end; ++i) {
        uint32_t prim_idx = build_infos.at(i).prim_index;
        this->ordered_prims.push_back(std::move(prims.at(prim_idx)));
      }
      return std::unique_ptr<BuildNode>(new BuildNode {
        bounds, { nullptr, nullptr },
        prims_begin, end - begin, axis,
      });
    }

    // make a branch
    uint32_t mid = 0;
    switch(split_method) {
      case BvhSplitMethod::Middle:
        mid = this->split_middle(build_infos, bounds, axis, begin, end);
        break;
      case BvhSplitMethod::EqualCounts:
        mid = this->split_equal_counts(build_infos, bounds, axis, begin, end);
        break;
      default:
        assert("Bad split method");
    }

    if(!(begin < mid && mid < end)) {
      mid = begin + (end - begin) / 2;
    }

    auto left = this->build_node(build_infos, prims,
        begin, mid, max_leaf_size, split_method, out_node_count);
    auto right = this->build_node(build_infos, prims,
        mid, end, max_leaf_size, split_method, out_node_count);
    return std::unique_ptr<BuildNode>(new BuildNode {
      bounds, { std::move(left), std::move(right) },
      prims_begin, end - begin, axis,
    });
  }

  uint32_t BvhPrimitive::split_middle(
      std::vector<PrimitiveInfo>& build_infos,
      const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end)
  {
    float center = (bounds.p_max + bounds.p_min).v[axis] * 0.5f;
    auto mid = std::partition(
        build_infos.begin() + begin,
        build_infos.begin() + end,
        [&](const PrimitiveInfo& prim_info) {
          return prim_info.centroid.v[axis] < center;
        });
    assert(mid > build_infos.begin());
    return mid - build_infos.begin();
  }

  uint32_t BvhPrimitive::split_equal_counts(
      std::vector<PrimitiveInfo>& build_infos,
      const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end)
  {
    (void)bounds;
    uint32_t mid = begin + (end - begin) / 2;
    std::nth_element(
        build_infos.begin() + begin,
        build_infos.begin() + mid,
        build_infos.begin() + end,
        [&](const PrimitiveInfo& i1, const PrimitiveInfo& i2) {
          return i1.centroid.v[axis] < i2.centroid.v[axis];
        });
    return mid;
  }

  void BvhPrimitive::linearize_node(const BuildNode& node, uint32_t depth)
  {
    uint32_t linear_index = this->linear_nodes.size();
    LinearNode linear_node;
    linear_node.bounds = node.bounds;
    linear_node.axis = node.axis;

    if(!node.children[0]) {
      // make a leaf
      assert(node.prims_length <= 0xff);
      linear_node.offset = node.prims_begin;
      linear_node.prim_count = node.prims_length;
      this->linear_nodes.push_back(linear_node);
      this->max_depth = max(this->max_depth, depth + 1);
    } else {
      // make a branch
      linear_node.prim_count = 0;
      linear_node.offset = -1u;
      this->linear_nodes.push_back(linear_node);
      this->linearize_node(*node.children[0], depth + 1);
      this->linear_nodes.at(linear_index).offset = this->linear_nodes.size();
      this->linearize_node(*node.children[1], depth + 1);
    }
  }

  bool BvhPrimitive::fast_box_hit_p(const Box& bounds, const Ray& ray,
      const Vector& inv_dir, bool dir_is_neg[3])
  {
    float t_min =  (bounds[  dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;
    float t_max =  (bounds[1-dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;

    float ty_min = (bounds[  dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    float ty_max = (bounds[1-dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    if(t_min > ty_max || t_max < ty_min) {
      return false;
    }

    if(ty_min > t_min) {
      t_min = ty_min;
    }
    if(t_max > ty_max) {
      t_max = ty_max;
    }

    float tz_min = (bounds[  dir_is_neg[2]].v.z - ray.orig.v.z) * inv_dir.v.z;
    float tz_max = (bounds[1-dir_is_neg[2]].v.z - ray.orig.v.z) * inv_dir.v.z;
    if(t_min > tz_max || t_max < tz_min) {
      return false;
    }

    if(tz_min > t_min) {
      t_min = tz_min;
    }
    if(t_max > tz_max) {
      t_max = tz_max;
    }

    return (t_min >= ray.t_min) && (t_max <= ray.t_max);
  }
}

#include <algorithm>
#include <array>
#include "dort/bvh_primitive.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  BvhPrimitive::BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
      uint32_t max_leaf_size, BvhSplitMethod split_method,
      ThreadPool& pool)
  {
    StatTimer t(TIMER_BVH_BUILD);

    std::vector<PrimitiveInfo> build_infos(prims.size());
    {
      StatTimer t(TIMER_BVH_COMPUTE_BUILD_INFOS);
      uint32_t jobs = std::min(pool.num_threads(),
          uint32_t(prims.size()) / MIN_PRIM_INFOS_PER_THREAD);
      fork_join(pool, jobs, [&](uint32_t job) {
        uint32_t begin = job * prims.size() / jobs;
        uint32_t end = (job + 1) * prims.size() / jobs;
        for(uint32_t i = begin; i < end; ++i) {
          Box bounds = prims.at(i)->bounds();
          assert(is_finite(bounds));
          build_infos.at(i) = PrimitiveInfo { i, bounds };
        }
      });
    }

    max_leaf_size = min(max_leaf_size, 0xffu);

    this->max_depth = 0;
    this->ordered_prims.reserve(prims.size());
    this->build_node(build_infos, prims,
        0, prims.size(), max_leaf_size, split_method, 1);
  }

  template<class F>
  void BvhPrimitive::traverse_primitives(const Ray& ray, F callback) const {
    Vector inv_dir(1.f / ray.dir.v.x, 1.f / ray.dir.v.y, 1.f / ray.dir.v.z);
    bool dir_is_neg[] = { ray.dir.v.x < 0.f, ray.dir.v.y < 0.f, ray.dir.v.z < 0.f };

    uint32_t todo_stack[this->max_depth + 1];
    uint32_t todo_top = 0;
    uint32_t todo_index = 0;
    uint32_t traversed = 0;
    for(;;) {
      StatTimer t_node(TIMER_BVH_TRAVERSE_NODE);
      const LinearNode& linear_node = this->linear_nodes.at(todo_index);
      traversed += 1;

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

        t_node.stop();
        for(uint32_t i = 0; i < linear_node.prim_count; ++i) {
          StatTimer t_prim(TIMER_BVH_INTERSECT_PRIM);
          auto& prim = this->ordered_prims.at(linear_node.offset + i);
          if(!callback(*prim)) {
            stat_sample_int(DISTRIB_INT_BVH_TRAVERSE_COUNT, traversed);
            return;
          }
        }
      }

      if(todo_top == 0) {
        break;
      }
      todo_index = todo_stack[--todo_top];
    }

    stat_sample_int(DISTRIB_INT_BVH_TRAVERSE_COUNT, traversed);
  }

  bool BvhPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    stat_count(COUNTER_BVH_INTERSECT);
    bool found = false;
    this->traverse_primitives(ray, [&](const Primitive& prim) {
      if(prim.intersect(ray, out_isect)) {
        found = true;
      }
      return true;
    });
    if(found) {
      stat_count(COUNTER_BVH_INTERSECT_HIT);
    }
    return found;
  }

  bool BvhPrimitive::intersect_p(const Ray& ray) const {
    stat_count(COUNTER_BVH_INTERSECT_P);
    bool found = false;
    this->traverse_primitives(ray, [&](const Primitive& prim) {
      if(prim.intersect_p(ray)) {
        found = true;
        return false;
      } else {
        return true;
      }
    });
    if(found) {
      stat_count(COUNTER_BVH_INTERSECT_P_HIT);
    }
    return found;
  }

  Box BvhPrimitive::bounds() const {
    if(this->linear_nodes.empty()) {
      return Box();
    }
    return this->linear_nodes.at(0).bounds;
  }

  uint32_t BvhPrimitive::build_node(
      std::vector<PrimitiveInfo>& build_infos,
      std::vector<std::unique_ptr<Primitive>>& prims,
      uint32_t begin, uint32_t end,
      uint32_t max_leaf_size, BvhSplitMethod split_method,
      uint32_t depth)
  {
    StatTimer t(TIMER_BVH_BUILD_NODE);
    stat_sample_int(DISTRIB_INT_BVH_BUILD_NODE_COUNT, end - begin);

    this->max_depth = std::max(this->max_depth, depth);
    uint32_t node_idx = this->linear_nodes.size();

    Box bounds;
    for(uint32_t i = begin; i < end; ++i) {
      bounds = union_box(bounds, build_infos.at(i).bounds);
    }

    uint8_t axis = box_max_axis(bounds);
    float extent = (bounds.p_max - bounds.p_min).v[axis];

    bool make_leaf = extent == 0.f || (end - begin) <= max_leaf_size;
    uint32_t mid;
    if(!make_leaf) {
      switch(split_method) {
        case BvhSplitMethod::Middle:
          mid = this->split_middle(build_infos, bounds, axis, begin, end);
          break;
        case BvhSplitMethod::Median:
          mid = this->split_median(build_infos, bounds, axis, begin, end);
          break;
        case BvhSplitMethod::Sah:
          mid = this->split_sah(build_infos, bounds, axis, begin, end);
          break;
        default:
          assert("Bad split method");
      }

      if(mid == -1u) {
        make_leaf = true;
      }
    }

    t.stop();

    if(make_leaf && (end - begin) <= max_leaf_size) {
      uint32_t prims_begin = this->ordered_prims.size();
      for(uint32_t i = begin; i < end; ++i) {
        uint32_t prim_idx = build_infos.at(i).prim_index;
        this->ordered_prims.push_back(std::move(prims.at(prim_idx)));
      }

      LinearNode leaf;
      leaf.bounds = bounds;
      leaf.offset = prims_begin;
      leaf.prim_count = end - begin;
      leaf.axis = axis;
      this->linear_nodes.push_back(leaf);
    } else {
      if(!(begin < mid && mid < end)) {
        mid = begin + (end - begin) / 2;
      }

      this->linear_nodes.emplace_back();
      uint32_t left_idx = this->build_node(build_infos, prims,
          begin, mid, max_leaf_size, split_method, depth + 1);
      uint32_t right_idx = this->build_node(build_infos, prims,
          mid, end, max_leaf_size, split_method, depth + 1);
      assert(left_idx == node_idx + 1); (void)left_idx;

      LinearNode& branch = this->linear_nodes.at(node_idx);
      branch.bounds = bounds;
      branch.offset = right_idx;
      branch.prim_count = 0;
      branch.axis = axis;
    }

    return node_idx;
  }

  uint32_t BvhPrimitive::split_middle(
      std::vector<PrimitiveInfo>& build_infos,
      const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end)
  {
    StatTimer t(TIMER_BVH_SPLIT_MIDDLE);
    float center = (bounds.p_max + bounds.p_min).v[axis] * 0.5f;
    auto mid = std::partition(
        build_infos.begin() + begin,
        build_infos.begin() + end,
        [&](const PrimitiveInfo& prim_info) {
          return prim_info.bounds.centroid().v[axis] < center;
        });
    return mid - build_infos.begin();
  }

  uint32_t BvhPrimitive::split_median(
      std::vector<PrimitiveInfo>& build_infos,
      const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end)
  {
    StatTimer t(TIMER_BVH_SPLIT_MEDIAN);
    (void)bounds;
    uint32_t mid = begin + (end - begin) / 2;
    std::nth_element(
        build_infos.begin() + begin,
        build_infos.begin() + mid,
        build_infos.begin() + end,
        [&](const PrimitiveInfo& i1, const PrimitiveInfo& i2) {
          return i1.bounds.centroid().v[axis] < i2.bounds.centroid().v[axis];
        });
    return mid;
  }

  uint32_t BvhPrimitive::split_sah(std::vector<PrimitiveInfo>& build_infos,
      const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end)
  {
    StatTimer t(TIMER_BVH_SPLIT_SAH);
    float extent = bounds.p_max.v[axis] - bounds.p_min.v[axis];
    float inv_extent = 1.f / extent;

    std::array<BucketInfo, SAH_BUCKET_COUNT> buckets;
    for(uint32_t i = begin; i < end; ++i) {
      auto& build_info = build_infos.at(i);
      uint32_t bucket_idx = clamp(uint32_t(floor_int32(float(SAH_BUCKET_COUNT) *
            (build_info.bounds.centroid().v[axis] - bounds.p_min.v[axis]) * inv_extent)),
          0u, SAH_BUCKET_COUNT - 1);
      auto& bucket = buckets.at(bucket_idx);
      bucket.count += 1;
      bucket.bounds = union_box(bucket.bounds, build_info.bounds);
    }

    Box prefix_bounds;
    uint32_t prefix_count = 0;
    for(uint32_t b = 0; b < buckets.size(); ++b) {
      prefix_bounds = union_box(prefix_bounds, buckets.at(b).bounds);
      prefix_count += buckets.at(b).count;
      buckets.at(b).prefix_bounds = prefix_bounds;
      buckets.at(b).prefix_count = prefix_count;
    }

    Box postfix_bounds;
    for(uint32_t b = buckets.size(); b-- > 0; ) {
      buckets.at(b).postfix_bounds = postfix_bounds;
      postfix_bounds = union_box(postfix_bounds, buckets.at(b).bounds);
    }

    int32_t best_cost = this->sah_split_cost(bounds, buckets.at(0), end - begin);
    uint32_t best_bucket = 0;
    for(uint32_t b = 1; b < buckets.size() - 1; ++b) {
      int32_t cost = this->sah_split_cost(bounds, buckets.at(b), end - begin);
      if(cost < best_cost) {
        best_cost = cost;
        best_bucket = b;
      }
    }

    int32_t leaf_cost = SAH_INTERSECTION_COST * (end - begin);
    if(best_cost < leaf_cost) {
      float boundary = float(best_bucket + 1) / float(buckets.size()) * extent 
        + bounds.p_min.v[axis];
      auto mid = std::partition(
          build_infos.begin() + begin,
          build_infos.begin() + end,
          [&](const PrimitiveInfo& prim_info) {
            return prim_info.bounds.centroid().v[axis] < boundary;
          });
      return mid - build_infos.begin();
    } else {
      return -1u;
    }
  }

  int32_t BvhPrimitive::sah_split_cost(const Box& bounds,
      const BucketInfo& bucket, uint32_t prim_count)
  {
    uint32_t count_l = bucket.prefix_count;
    uint32_t count_r = prim_count - count_l;
    if(count_l == 0 || count_r == 0) {
      return INT32_MAX;
    }

    float area_l = bucket.prefix_bounds.area();
    float area_r = bucket.postfix_bounds.area();
    float isects = (area_l * float(count_l) + area_r * float(count_r)) / bounds.area();
    return SAH_TRAVERSAL_COST + floor_int32(isects * float(SAH_INTERSECTION_COST));
  }

  bool BvhPrimitive::fast_box_hit_p(const Box& bounds, const Ray& ray,
      const Vector& inv_dir, bool dir_is_neg[3])
  {
    stat_count(COUNTER_BVH_FAST_BOX_INTERSECT_P);

    float t_min =  (bounds[  dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;
    float t_max =  (bounds[1-dir_is_neg[0]].v.x - ray.orig.v.x) * inv_dir.v.x;

    float ty_min = (bounds[  dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    float ty_max = (bounds[1-dir_is_neg[1]].v.y - ray.orig.v.y) * inv_dir.v.y;
    if(t_min > ty_max || ty_min > t_max) {
      return false;
    }

    if(ty_min > t_min) {
      t_min = ty_min;
    }
    if(ty_max < t_max) {
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

    bool hit = (t_min < ray.t_max) && (t_max > ray.t_min);
    if(hit) {
      stat_count(COUNTER_BVH_FAST_BOX_INTERSECT_P_HIT);
    }
    return hit;
  }
}

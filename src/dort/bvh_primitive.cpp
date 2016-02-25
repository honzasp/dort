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
    Box root_bounds;
    Box root_centroid_bounds;
    auto build_infos = this->compute_build_infos(prims,
        root_bounds, root_centroid_bounds, pool);

    BuildCtx ctx {
      std::move(build_infos),
      std::move(prims),
      pool,
      std::min(max_leaf_size, 0xffu),
      split_method,
      {},
      {},
      {},
    };
    ctx.free_linear_idx.store(1);

    NodeInfo root_node {
      root_bounds,
      root_centroid_bounds,
      0, uint32_t(ctx.prims.size()),
      0
    };

    this->ordered_prims.resize(ctx.prims.size());
    this->linear_nodes.resize(ctx.prims.size() / max_leaf_size);
    this->build_node(ctx, root_node, true);

    std::sort(ctx.todo_serial.begin(), ctx.todo_serial.end(),
        [&](const NodeInfo& n1, const NodeInfo& n2) {
          return (n1.end - n1.begin) > (n2.end - n2.begin);
        });
    stat_sample_int(DISTRIB_INT_BVH_BUILD_SERIAL_COUNT, ctx.todo_serial.size());

    fork_join(ctx.pool, ctx.todo_serial.size(), [&](uint32_t job) {
      this->build_node(ctx, ctx.todo_serial.at(job), false);
    });

    this->linear_nodes.shrink_to_fit();
  }

  std::vector<BvhPrimitive::PrimitiveInfo> BvhPrimitive::compute_build_infos(
      const std::vector<std::unique_ptr<Primitive>>& prims,
      Box& out_bounds, Box& out_centroid_bounds,
      ThreadPool& pool)
  {
    StatTimer t(TIMER_BVH_COMPUTE_BUILD_INFOS);
    std::vector<PrimitiveInfo> build_infos(prims.size());
    uint32_t jobs = std::min(pool.num_threads(),
        std::max(1u, uint32_t(prims.size()) / MIN_PRIM_INFOS_PER_THREAD));

    // TODO: use small_vector
    std::vector<Box> job_bounds(jobs);
    std::vector<Box> job_centroid_bounds(jobs);
    fork_join(pool, jobs, [&](uint32_t job) {
      uint32_t begin = uint64_t(job) * prims.size() / jobs;
      uint32_t end = uint64_t(job + 1) * prims.size() / jobs;
      Box bounds, centroid_bounds;

      for(uint32_t i = begin; i < end; ++i) {
        Box prim_bounds = prims.at(i)->bounds();
        Point centroid = 0.5f * (prim_bounds.p_min + prim_bounds.p_max);
        assert(is_finite(prim_bounds));

        build_infos.at(i) = PrimitiveInfo { i, prim_bounds };
        bounds = union_box(bounds, prim_bounds);
        centroid_bounds = union_box(centroid_bounds, centroid);
      }

      job_bounds.at(job) = bounds;
      job_centroid_bounds.at(job) = centroid_bounds;
    });

    for(uint32_t i = 0; i < jobs; ++i) {
      out_bounds = union_box(out_bounds, job_bounds.at(i));
      out_centroid_bounds = union_box(out_centroid_bounds, job_bounds.at(i));
    }

    return build_infos;
  }

  void BvhPrimitive::build_node(BuildCtx& ctx, const NodeInfo& node, bool parallel) {
    uint32_t prim_count = node.end - node.begin;
    StatTimer t(parallel
        ? TIMER_BVH_BUILD_NODE_PARALLEL
        : TIMER_BVH_BUILD_NODE_SERIAL);
    stat_sample_int(parallel 
        ? DISTRIB_INT_BVH_BUILD_NODE_PARALLEL_COUNT 
        : DISTRIB_INT_BVH_BUILD_NODE_SERIAL_COUNT, prim_count);

    uint32_t min_para_prims = ctx.pool.num_threads() * MIN_SPLIT_PRIMS_PER_THREAD;
    bool parallel_split = prim_count > min_para_prims;
    assert(parallel_split ? parallel : true);
    if(parallel && !parallel_split) {
      ctx.todo_serial.push_back(node);
      return;
    }

    uint8_t axis = node.centroid_bounds.max_axis();
    float extent = (node.bounds.p_max - node.bounds.p_min).v[axis];
    bool make_leaf = extent == 0.f || prim_count <= ctx.max_leaf_size;

    SplitInfo split;
    if(!make_leaf) {
      switch(ctx.split_method) {
        case BvhSplitMethod::Middle:
          split = this->split_middle(ctx, node, axis, parallel_split);
          break;
        case BvhSplitMethod::Sah:
          split = this->split_sah(ctx, node, axis, parallel_split);
          break;
        default:
          assert("Bad split method");
      }

      if(split.mid == -1u) {
        make_leaf = true;
      }

      assert(node.begin < split.mid && split.mid + 1 < node.end);
    }

    if(make_leaf && prim_count <= ctx.max_leaf_size) {
      for(uint32_t i = node.begin; i < node.end; ++i) {
        uint32_t prim_idx = ctx.build_infos.at(i).prim_index;
        this->ordered_prims.at(i) = std::move(ctx.prims.at(prim_idx));
      }

      LinearNode leaf;
      leaf.bounds = node.bounds;
      leaf.prim_offset_or_left_child = node.begin;
      leaf.prim_count_or_zero = prim_count;
      leaf.axis = axis;
      this->write_linear_node(ctx, node.linear_idx, leaf);
    } else {
      NodeInfo left;
      left.bounds = split.left_bounds;
      left.centroid_bounds = split.left_centroid_bounds;
      left.begin = node.begin;
      left.end = split.mid;
      left.linear_idx = ctx.free_linear_idx.fetch_add(2);

      NodeInfo right;
      right.bounds = split.right_bounds;
      right.centroid_bounds = split.right_centroid_bounds;
      right.begin = split.mid;
      right.end = node.end;
      right.linear_idx = left.linear_idx + 1;

      LinearNode branch;
      branch.bounds = node.bounds;
      branch.prim_offset_or_left_child = left.linear_idx;
      branch.prim_count_or_zero = 0;
      branch.axis = axis;
      this->write_linear_node(ctx, node.linear_idx, branch);

      t.stop();
      this->build_node(ctx, left, parallel);
      this->build_node(ctx, right, parallel);
    }
  }

  void BvhPrimitive::write_linear_node(BuildCtx& ctx, uint32_t idx,
      const LinearNode& node)
  {
    StatTimer t(TIMER_BVH_WRITE_LINEAR_NODE);
    std::shared_lock<std::shared_timed_mutex> write_lock(ctx.linear_mutex);
    if(this->linear_nodes.size() <= idx) {
      StatTimer t(TIMER_BVH_WRITE_LINEAR_NODE_RESIZE);
      write_lock.unlock();
      std::unique_lock<std::shared_timed_mutex> resize_lock(ctx.linear_mutex);
      this->linear_nodes.reserve(std::max(idx + 1, idx * 2 - idx / 2));
      uint32_t capacity = this->linear_nodes.capacity();
      if(this->linear_nodes.size() < capacity) {
        stat_sample_int(DISTRIB_INT_BVH_BUILD_LINEAR_RESIZE, capacity);
        this->linear_nodes.resize(capacity);
      }
      resize_lock.unlock();
      write_lock.lock();
    }

    this->linear_nodes.at(idx) = node;
  }

  BvhPrimitive::SplitInfo BvhPrimitive::split_middle(BuildCtx& ctx,
      const NodeInfo& node, uint8_t axis, bool parallel)
  {
    StatTimer t(TIMER_BVH_SPLIT_MIDDLE);
    uint32_t begin = node.begin;
    uint32_t end = node.end;
    float center = node.centroid_bounds.centroid().v[axis];
    uint32_t mid = this->partition(ctx, parallel, begin, end, axis, center);
    if(mid == begin || mid + 1 == end) {
      mid = begin + (end - begin) / 2;
    }

    uint32_t jobs = !parallel ? 1
      : std::max(ctx.pool.num_threads(),
          (end - begin) / MIN_SPLIT_PRIMS_PER_THREAD);
    // TODO: use small_vector (to optimize non-parallel splits)
    std::vector<Box> job_left_bounds(jobs);
    std::vector<Box> job_left_centroid_bounds(jobs);
    std::vector<Box> job_right_bounds(jobs);
    std::vector<Box> job_right_centroid_bounds(jobs);

    StatTimer t_out(parallel
        ? TIMER_BVH_SPLIT_MIDDLE_BOUNDS_OUT_PARALLEL
        : TIMER_BVH_SPLIT_MIDDLE_BOUNDS_OUT_SERIAL);
    if(parallel) {
      stat_sample_int(DISTRIB_INT_BVH_SPLIT_MIDDLE_JOBS, jobs);
    }

    fork_join_or_serial(ctx.pool, !parallel, jobs, [&](uint32_t job) {
      StatTimer t_in(parallel
          ? TIMER_BVH_SPLIT_MIDDLE_BOUNDS_IN_PARALLEL
          : TIMER_BVH_SPLIT_MIDDLE_BOUNDS_IN_SERIAL);
      uint32_t left_begin = begin + uint64_t(job) * (mid - begin) / jobs;
      uint32_t left_end = begin + uint64_t(job + 1) * (mid - begin) / jobs;
      uint32_t right_begin = mid + uint64_t(job) * (end - mid) / jobs;
      uint32_t right_end = mid + uint64_t(job + 1) * (end - mid) / jobs;

      Box left_bounds, left_centroid_bounds;
      for(uint32_t i = left_begin; i < left_end; ++i) {
        Box bounds = ctx.build_infos.at(i).bounds;
        left_bounds = union_box(left_bounds, bounds);
        left_centroid_bounds = union_box(left_centroid_bounds, bounds.centroid());
      }

      Box right_bounds, right_centroid_bounds;
      for(uint32_t i = right_begin; i < right_end; ++i) {
        Box bounds = ctx.build_infos.at(i).bounds;
        right_bounds = union_box(right_bounds, bounds);
        right_centroid_bounds = union_box(right_centroid_bounds, bounds.centroid());
      }

      job_left_bounds.at(job) = left_bounds;
      job_left_centroid_bounds.at(job) = left_centroid_bounds;
      job_right_bounds.at(job) = right_bounds;
      job_right_centroid_bounds.at(job) = right_centroid_bounds;
    });

    Box left_bounds, left_centroid_bounds;
    Box right_bounds, right_centroid_bounds;
    for(uint32_t i = 0; i < jobs; ++i) {
      left_bounds = union_box(left_bounds, job_left_bounds.at(i));
      left_centroid_bounds = union_box(left_centroid_bounds,
          job_left_centroid_bounds.at(i));
      right_bounds = union_box(right_bounds, job_right_bounds.at(i));
      right_centroid_bounds = union_box(right_centroid_bounds,
          job_right_centroid_bounds.at(i));
    }

    SplitInfo split;
    split.left_bounds = left_bounds;
    split.right_bounds = right_bounds;
    split.left_centroid_bounds = left_centroid_bounds;
    split.right_centroid_bounds = right_centroid_bounds;
    split.mid = mid;
    return split;
  }

  BvhPrimitive::SplitInfo BvhPrimitive::split_sah(BuildCtx& ctx,
      const NodeInfo& node, uint8_t axis, bool parallel)
  {
    (void)ctx; (void)node; (void)axis; (void)parallel;
    assert("sah not implemented");
    return SplitInfo();
  }

  uint32_t BvhPrimitive::partition(BuildCtx& ctx, bool parallel,
      uint32_t begin, uint32_t end, uint8_t axis, float separator)
  {
    StatTimer t(parallel
        ? TIMER_BVH_BUILD_PARTITION_PARALLEL
        : TIMER_BVH_BUILD_PARTITION_SERIAL);
    auto mid = std::partition(
        ctx.build_infos.begin() + begin,
        ctx.build_infos.begin() + end,
        [&](const PrimitiveInfo& info) {
          float centroid = (info.bounds.p_max.v[axis] + info.bounds.p_min.v[axis]) * 0.5f;
          return centroid < separator;
        });
    return mid - ctx.build_infos.begin();
  }

  /*
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
  */

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

  template<class F>
  void BvhPrimitive::traverse_primitives(const Ray& ray, F callback) const {
    Vector inv_dir(1.f / ray.dir.v.x, 1.f / ray.dir.v.y, 1.f / ray.dir.v.z);
    bool dir_is_neg[] = { ray.dir.v.x < 0.f, ray.dir.v.y < 0.f, ray.dir.v.z < 0.f };

    std::array<uint32_t, 64> todo_stack;
    uint32_t todo_top = 0;
    uint32_t todo_index = 0;
    uint32_t traversed = 0;
    for(;;) {
      StatTimer t_node(TIMER_BVH_TRAVERSE_NODE);
      const LinearNode& linear_node = this->linear_nodes.at(todo_index);
      traversed += 1;

      if(fast_box_hit_p(linear_node.bounds, ray, inv_dir, dir_is_neg)) {
        if(linear_node.prim_count_or_zero == 0) {
          uint32_t left_child = linear_node.prim_offset_or_left_child;
          uint32_t right_child = left_child + 1;
          if(dir_is_neg[linear_node.axis]) {
            todo_stack.at(todo_top++) = left_child;
            todo_index = right_child;
          } else {
            todo_stack.at(todo_top++) = right_child;
            todo_index = left_child;
          }
          continue;
        }

        t_node.stop();
        for(uint32_t i = 0; i < linear_node.prim_count_or_zero; ++i) {
          StatTimer t_prim(TIMER_BVH_INTERSECT_PRIM);
          auto& prim = this->ordered_prims.at(linear_node.prim_offset_or_left_child + i);
          if(!callback(*prim)) {
            stat_sample_int(DISTRIB_INT_BVH_TRAVERSE_COUNT, traversed);
            return;
          }
        }
      }

      if(todo_top == 0) {
        break;
      }
      todo_index = todo_stack.at(--todo_top);
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
}

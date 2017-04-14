#include <algorithm>
#include "dort/bvh.hpp"
#include "dort/bvh_primitive.hpp"
#include "dort/mesh_bvh_primitive.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  template<class R>
  Bvh<R>::Bvh(std::vector<Element> elems, TraitsArg arg,
      const BvhOpts& opts, ThreadPool& pool) 
  {
    StatTimer t(TIMER_BVH_BUILD);
    Box root_bounds;
    Box root_centroid_bounds;
    auto build_infos = Bvh::compute_build_infos(elems,
        root_bounds, root_centroid_bounds, arg, opts, pool);

    BuildCtx ctx {
      std::move(build_infos),
      std::move(elems),
      pool,
      opts,
      {}, {}, {}, {}, {},
    };
    ctx.opts.max_leaf_size = std::min(ctx.opts.max_leaf_size, 0xffffu);
    ctx.opts.leaf_size = std::min(ctx.opts.leaf_size, ctx.opts.max_leaf_size);
    ctx.free_linear_idx.store(1);
    ctx.ordered_elems.resize(ctx.elems.size());
    ctx.linear_nodes.resize(ctx.elems.size() / ctx.opts.leaf_size);

    NodeInfo root_node {
      root_bounds,
      root_centroid_bounds,
      0, uint32_t(ctx.elems.size()),
      0
    };

    Bvh::build_node(ctx, root_node, true);

    std::sort(ctx.todo_serial.begin(), ctx.todo_serial.end(),
        [&](const NodeInfo& n1, const NodeInfo& n2) {
          return (n1.end - n1.begin) > (n2.end - n2.begin);
        });
    stat_sample_int(DISTRIB_INT_BVH_BUILD_SERIAL_COUNT, ctx.todo_serial.size());

    fork_join(ctx.pool, ctx.todo_serial.size(), [&](uint32_t job) {
        Bvh::build_node(ctx, ctx.todo_serial.at(job), false);
    });

    ctx.linear_nodes.resize(ctx.free_linear_idx.load());
    ctx.linear_nodes.shrink_to_fit();
    this->linear_nodes = std::move(ctx.linear_nodes);
    this->ordered_elems = std::move(ctx.ordered_elems);
  }

  template<class R>
  std::vector<typename Bvh<R>::ElementInfo> Bvh<R>::compute_build_infos(
      const std::vector<Element>& elems,
      Box& out_bounds, Box& out_centroid_bounds,
      TraitsArg traits_arg,
      const BvhOpts& opts, ThreadPool& pool)
  {
    StatTimer t(TIMER_BVH_COMPUTE_BUILD_INFOS);
    std::vector<ElementInfo> build_infos(elems.size());
    uint32_t jobs = std::min(pool.thread_count(),
        std::max(1u, uint32_t(elems.size()) / opts.min_elem_infos_per_thread));

    // TODO: use small_vector
    std::vector<Box> job_bounds(jobs);
    std::vector<Box> job_centroid_bounds(jobs);
    fork_join(pool, jobs, [&](uint32_t job) {
      uint32_t begin = uint64_t(job) * elems.size() / jobs;
      uint32_t end = uint64_t(job + 1) * elems.size() / jobs;
      Box bounds, centroid_bounds;

      for(uint32_t i = begin; i < end; ++i) {
        Box elem_bounds = Bvh::get_elem_bounds(traits_arg, elems.at(i));
        assert(is_finite(elem_bounds));

        build_infos.at(i) = ElementInfo { i, elem_bounds };
        bounds = union_box(bounds, elem_bounds);
        centroid_bounds = union_box(centroid_bounds, elem_bounds.centroid());
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

  template<class R>
  void Bvh<R>::build_node(BuildCtx& ctx, const NodeInfo& node, bool parallel) {
    uint32_t elem_count = node.end - node.begin;
    StatTimer t(parallel
        ? TIMER_BVH_BUILD_NODE_PARALLEL
        : TIMER_BVH_BUILD_NODE_SERIAL);
    stat_sample_int(parallel 
        ? DISTRIB_INT_BVH_BUILD_NODE_PARALLEL_COUNT 
        : DISTRIB_INT_BVH_BUILD_NODE_SERIAL_COUNT, elem_count);

    uint32_t min_para_elems = ctx.pool.thread_count() 
      * ctx.opts.min_split_elems_per_thread;
    bool parallel_split = elem_count > min_para_elems;
    assert(parallel_split ? parallel : true);
    if(parallel && !parallel_split) {
      ctx.todo_serial.push_back(node);
      return;
    }

    uint8_t axis = node.centroid_bounds.max_axis();
    bool make_leaf = elem_count <= ctx.opts.leaf_size;

    SplitInfo split;
    if(!make_leaf) {
      switch(ctx.opts.split_method) {
        case BvhSplitMethod::Middle:
          split = Bvh::split_middle(ctx, node, axis, parallel_split);
          break;
        case BvhSplitMethod::Sah:
          split = Bvh::split_sah(ctx, node, axis, parallel_split);
          break;
        default:
          assert("Bad split method");
      }

      make_leaf = split.prefer_leaf && elem_count <= ctx.opts.max_leaf_size;
      assert(node.begin < split.mid && split.mid + 1 < node.end);
    }

    if(make_leaf) {
      assert(elem_count <= ctx.opts.max_leaf_size);
      for(uint32_t i = node.begin; i < node.end; ++i) {
        uint32_t elem_idx = ctx.build_infos.at(i).elem_index;
        ctx.ordered_elems.at(i) = std::move(ctx.elems.at(elem_idx));
      }

      LinearNode leaf;
      leaf.bounds = node.bounds;
      leaf.elem_offset_or_left_child = node.begin;
      leaf.elem_count_or_zero = elem_count;
      leaf.axis = axis;
      Bvh::write_linear_node(ctx, node.linear_idx, leaf);
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
      branch.elem_offset_or_left_child = left.linear_idx;
      branch.elem_count_or_zero = 0;
      branch.axis = axis;
      Bvh::write_linear_node(ctx, node.linear_idx, branch);

      t.stop();
      Bvh::build_node(ctx, left, parallel);
      Bvh::build_node(ctx, right, parallel);
    }
  }

  template<class R>
  void Bvh<R>::write_linear_node(BuildCtx& ctx, uint32_t idx,
      const LinearNode& node)
  {
    StatTimer t(TIMER_BVH_WRITE_LINEAR_NODE);
    std::shared_lock<std::shared_timed_mutex> write_lock(ctx.linear_mutex);
    if(ctx.linear_nodes.size() <= idx) {
      StatTimer t(TIMER_BVH_WRITE_LINEAR_NODE_RESIZE);
      write_lock.unlock();
      std::unique_lock<std::shared_timed_mutex> resize_lock(ctx.linear_mutex);
      ctx.linear_nodes.reserve(std::max(idx + 1, idx * 2 - idx / 2));
      uint32_t capacity = ctx.linear_nodes.capacity();
      if(ctx.linear_nodes.size() < capacity) {
        stat_sample_int(DISTRIB_INT_BVH_BUILD_LINEAR_RESIZE, capacity);
        ctx.linear_nodes.resize(capacity);
      }
      resize_lock.unlock();
      write_lock.lock();
    }

    ctx.linear_nodes.at(idx) = node;
  }

  template<class R>
  typename Bvh<R>::SplitInfo Bvh<R>::split_middle(BuildCtx& ctx,
      const NodeInfo& node, uint8_t axis, bool parallel)
  {
    StatTimer t(TIMER_BVH_SPLIT_MIDDLE);
    uint32_t begin = node.begin;
    uint32_t end = node.end;
    float center = node.centroid_bounds.centroid().v[axis];
    uint32_t mid = Bvh::partition(ctx, parallel, begin, end, axis, center);
    if(mid == begin || mid + 1 == end) {
      mid = begin + (end - begin) / 2;
    }

    uint32_t jobs = !parallel ? 1
      : std::max(ctx.pool.thread_count(),
          (end - begin) / ctx.opts.min_split_elems_per_thread);
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
    split.prefer_leaf = false;
    return split;
  }

  template<class R>
  typename Bvh<R>::SplitInfo Bvh<R>::split_sah(BuildCtx& ctx,
      const NodeInfo& node, uint8_t axis, bool parallel)
  {
    StatTimer t(TIMER_BVH_SPLIT_SAH);
    float extent = (node.centroid_bounds.p_max - node.centroid_bounds.p_min).v[axis];
    float bounds_min = node.centroid_bounds.p_min.v[axis];
    float inv_extent = 1.f / extent;
    uint32_t begin = node.begin;
    uint32_t end = node.end;
    uint32_t bucket_count = ctx.opts.sah_bucket_count;

    uint32_t jobs = !parallel ? 1
      : std::max(ctx.pool.thread_count(),
          (end - begin) / ctx.opts.min_split_elems_per_thread);

    std::vector<std::vector<BucketInfo>> job_buckets(jobs);
    fork_join_or_serial(ctx.pool, !parallel, jobs, [&](uint32_t job) {
      std::vector<BucketInfo> buckets(bucket_count);
      uint32_t job_begin = begin + uint64_t(end - begin) * job / jobs;
      uint32_t job_end = begin + uint64_t(end - begin) * (job + 1) / jobs;
      for(uint32_t i = job_begin; i < job_end; ++i) {
        auto& elem_info = ctx.build_infos.at(i);
        Point centroid = elem_info.bounds.centroid();
        float elem_pos = centroid.v[axis] - bounds_min;
        uint32_t bucket_idx = clamp(
          uint32_t(floor_int32(float(bucket_count) * elem_pos * inv_extent)),
          0u, bucket_count - 1);
        auto& bucket = buckets.at(bucket_idx);
        bucket.count += 1;
        bucket.bounds = union_box(bucket.bounds, elem_info.bounds);
        bucket.centroid_bounds = union_box(bucket.centroid_bounds, centroid);
      }
      job_buckets.at(job) = std::move(buckets);
    });

    std::vector<BucketInfo> buckets(bucket_count);
    for(uint32_t j = 0; j < jobs; ++j) {
      for(uint32_t b = 0; b < buckets.size(); ++b) {
        auto& bucket = buckets.at(b);
        auto& job_bucket = job_buckets.at(j).at(b);
        bucket.count += job_bucket.count;
        bucket.bounds = union_box(bucket.bounds, job_bucket.bounds);
        bucket.centroid_bounds = union_box(bucket.centroid_bounds,
            job_bucket.centroid_bounds);
      }
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

    int32_t best_cost = Bvh::sah_split_cost(node.centroid_bounds,
        buckets.at(0), end - begin);
    uint32_t best_bucket = 0;
    for(uint32_t b = 1; b < buckets.size() - 1; ++b) {
      int32_t cost = Bvh::sah_split_cost(node.centroid_bounds,
          buckets.at(b), end - begin);
      if(cost < best_cost) {
        best_cost = cost;
        best_bucket = b;
      }
    }

    int32_t leaf_cost = SAH_INTERSECTION_COST * (end - begin);
    float separator = float(best_bucket + 1) / float(buckets.size()) * extent 
      + bounds_min;

    SplitInfo split;
    split.prefer_leaf = leaf_cost < best_cost;
    split.mid = Bvh::partition(ctx,
        parallel, begin, end, axis, separator);

    for(uint32_t b = 0; b < buckets.size(); ++b) {
      if(b <= best_bucket) {
        split.left_bounds = union_box(split.left_bounds, buckets.at(b).bounds);
        split.left_centroid_bounds = union_box(split.left_centroid_bounds,
            buckets.at(b).centroid_bounds);
      } else {
        split.right_bounds = union_box(split.right_bounds, buckets.at(b).bounds);
        split.right_centroid_bounds = union_box(split.right_centroid_bounds,
            buckets.at(b).centroid_bounds);
      }
    }

    if(split.mid == begin || split.mid + 1 == end) {
      split.mid = begin + (end - begin) / 2;
      split.left_bounds = split.right_bounds = node.bounds;
      split.left_centroid_bounds = split.right_centroid_bounds = node.centroid_bounds;
    }

    return split;
  }

  template<class R>
  uint32_t Bvh<R>::partition(BuildCtx& ctx, bool parallel,
      uint32_t begin, uint32_t end, uint8_t axis, float separator)
  {
    StatTimer t(parallel
        ? TIMER_BVH_BUILD_PARTITION_PARALLEL
        : TIMER_BVH_BUILD_PARTITION_SERIAL);
    auto mid = std::partition(
        ctx.build_infos.begin() + begin,
        ctx.build_infos.begin() + end,
        [&](const ElementInfo& info) {
          return info.bounds.centroid().v[axis] < separator;
        });
    return mid - ctx.build_infos.begin();
  }

  template<class R>
  int32_t Bvh<R>::sah_split_cost(const Box& centroid_bounds,
      const BucketInfo& bucket, uint32_t elem_count)
  {
    uint32_t count_l = bucket.prefix_count;
    uint32_t count_r = elem_count - count_l;
    if(count_l == 0 || count_r == 0) {
      return INT32_MAX;
    }

    float area_l = bucket.prefix_bounds.area();
    float area_r = bucket.postfix_bounds.area();
    float isects = (area_l * float(count_l) + area_r * float(count_r)) /
      centroid_bounds.area();
    return SAH_TRAVERSAL_COST + floor_int32(isects * float(SAH_INTERSECTION_COST));
  }

  template<class R>
  void Bvh<R>::traverse_elems(const Ray& ray,
      std::function<bool(const Element&)> callback) const 
  {
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

      stat_count(COUNTER_BVH_FAST_BOX_INTERSECT_P);
      if(linear_node.bounds.fast_hit_p(ray, inv_dir, dir_is_neg)) {
        stat_count(COUNTER_BVH_FAST_BOX_INTERSECT_P_HIT);
        if(linear_node.elem_count_or_zero == 0) {
          uint32_t left_child = linear_node.elem_offset_or_left_child;
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
        for(uint32_t i = 0; i < linear_node.elem_count_or_zero; ++i) {
          StatTimer t_elem(TIMER_BVH_TRAVERSE_ELEM);
          uint32_t elem_idx = linear_node.elem_offset_or_left_child + i;
          const auto& elem = this->ordered_elems.at(elem_idx);
          if(!callback(elem)) {
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

  template<class R>
  Box Bvh<R>::bounds() const {
    if(this->linear_nodes.empty()) {
      return Box();
    }
    return this->linear_nodes.at(0).bounds;
  }

  template class Bvh<BvhPrimitive::BvhTraits>;
  template class Bvh<MeshBvhPrimitive::BvhTraits>;
}

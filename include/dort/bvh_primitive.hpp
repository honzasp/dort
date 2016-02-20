#pragma once
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <vector>
#include "dort/primitive.hpp"

namespace dort {
  enum class BvhSplitMethod {
    Middle,
    Sah,
  };

  class BvhPrimitive final: public Primitive {
    static constexpr uint32_t MIN_PRIM_INFOS_PER_THREAD = 5*1000; // TODO
    static constexpr uint32_t MIN_SPLIT_PRIMS_PER_THREAD = 2*1000; // TODO
    static constexpr uint32_t SAH_BUCKET_COUNT = 12;
    static constexpr int32_t SAH_INTERSECTION_COST = 2;
    static constexpr int32_t SAH_TRAVERSAL_COST = 1;

    struct LinearNode {
      Box bounds;
      uint32_t prim_offset_or_left_child;
      uint8_t prim_count_or_zero;
      uint8_t axis;
      uint8_t padding[2];
    };

    struct PrimitiveInfo {
      uint32_t prim_index;
      Box bounds;
    };

    struct BucketInfo {
      uint32_t count = 0;
      Box bounds;
      Box prefix_bounds;
      Box postfix_bounds;
      uint32_t prefix_count;
    };

    struct NodeInfo {
      Box bounds;
      Box centroid_bounds;
      uint32_t begin;
      uint32_t end;
      uint32_t linear_idx;
    };

    struct SplitInfo {
      Box left_bounds, right_bounds;
      Box left_centroid_bounds, right_centroid_bounds;
      uint32_t mid;
    };

    struct BuildCtx {
      std::vector<PrimitiveInfo> build_infos;
      std::vector<std::unique_ptr<Primitive>> prims;
      ThreadPool& pool;
      uint32_t max_leaf_size;
      BvhSplitMethod split_method;
      std::atomic<uint32_t> free_linear_idx;
      std::vector<NodeInfo> todo_serial;
      std::shared_timed_mutex linear_mutex;
    };

    std::vector<LinearNode> linear_nodes;
    std::vector<std::unique_ptr<Primitive>> ordered_prims;
  public:
    BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
        uint32_t max_leaf_size, BvhSplitMethod split_method,
        ThreadPool& pool);
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const final override;
    virtual Box bounds() const override final;
  private:
    std::vector<PrimitiveInfo> compute_build_infos(
        const std::vector<std::unique_ptr<Primitive>>& prims,
        Box& out_bounds, Box& out_centroid_bounds,
        ThreadPool& pool);

    void build_node(BuildCtx& ctx, const NodeInfo& node, bool parallel);
    void write_linear_node(BuildCtx& ctx, uint32_t idx, const LinearNode& node);

    SplitInfo split_middle(BuildCtx& ctx, const NodeInfo& node,
        uint8_t axis, bool parallel);
    SplitInfo split_sah(BuildCtx& ctx, const NodeInfo& node,
        uint8_t axis, bool parallel);
    uint32_t partition(BuildCtx& ctx, bool parallel,
        uint32_t begin, uint32_t end, uint8_t axis, float separator);

    int32_t sah_split_cost(const Box& bounds,
        const BucketInfo& bucket, uint32_t prim_count);

    template<class F>
    void traverse_primitives(const Ray& ray, F callback) const;

    static bool fast_box_hit_p(const Box& bounds, const Ray& ray,
        const Vector& inv_dir, bool dir_is_neg[3]);
  };
}

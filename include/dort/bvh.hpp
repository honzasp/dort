#pragma once
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <vector>
#include "dort/geometry.hpp"

namespace dort {
  enum class BvhSplitMethod {
    Middle,
    Sah,
  };

  struct BvhOpts {
    BvhSplitMethod split_method = BvhSplitMethod::Middle;
    uint32_t leaf_size = 6;
    uint32_t max_leaf_size = 32;
    uint32_t min_elem_infos_per_thread = 5*1000;
    uint32_t min_split_elems_per_thread = 200*1000;
    uint32_t sah_bucket_count = 12;
  };

  template<class Traits>
  class Bvh final {
    using Element = typename Traits::Element;
    using TraitsArg = typename Traits::Arg;
    static Box get_elem_bounds(TraitsArg arg, const Element& elem) {
      return Traits::get_bounds(arg, elem);
    }

    static constexpr int32_t SAH_INTERSECTION_COST = 2;
    static constexpr int32_t SAH_TRAVERSAL_COST = 1;

    struct LinearNode {
      Box bounds;
      uint32_t elem_offset_or_left_child;
      uint16_t elem_count_or_zero;
      uint8_t axis;
      uint8_t padding[1];
    };

    struct ElementInfo {
      uint32_t elem_index;
      Box bounds;
    };

    struct BucketInfo {
      uint32_t count = 0;
      Box bounds;
      Box centroid_bounds;
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
      bool prefer_leaf;
    };

    struct BuildCtx {
      std::vector<ElementInfo> build_infos;
      std::vector<Element> elems;
      ThreadPool& pool;
      BvhOpts opts;
      std::atomic<uint32_t> free_linear_idx;
      std::vector<NodeInfo> todo_serial;
      std::shared_timed_mutex linear_mutex;
      std::vector<Element> ordered_elems;
      std::vector<LinearNode> linear_nodes;
    };

    std::vector<LinearNode> linear_nodes;
    std::vector<Element> ordered_elems;
  public:
    Bvh(std::vector<Element> elems, TraitsArg arg,
        const BvhOpts& opts, ThreadPool& pool);
    Box bounds() const;
    void traverse_elems(const Ray& ray,
        std::function<bool(const Element&)> callback) const;
  private:
    static std::vector<ElementInfo> compute_build_infos(
        const std::vector<Element>& elems,
        Box& out_bounds, Box& out_centroid_bounds,
        TraitsArg traits_arg,
        const BvhOpts& opts, ThreadPool& pool);

    static void build_node(BuildCtx& ctx, const NodeInfo& node, bool parallel);
    static void write_linear_node(BuildCtx& ctx,
        uint32_t idx, const LinearNode& node);

    static SplitInfo split_middle(BuildCtx& ctx, const NodeInfo& node,
        uint8_t axis, bool parallel);
    static SplitInfo split_sah(BuildCtx& ctx, const NodeInfo& node,
        uint8_t axis, bool parallel);
    static uint32_t partition(BuildCtx& ctx, bool parallel,
        uint32_t begin, uint32_t end, uint8_t axis, float separator);

    static int32_t sah_split_cost(const Box& centroid_bounds,
        const BucketInfo& bucket, uint32_t elem_count);
  };
}

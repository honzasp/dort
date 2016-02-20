#pragma once
#include <memory>
#include <vector>
#include "dort/primitive.hpp"

namespace dort {
  enum class BvhSplitMethod {
    Middle,
    EqualCounts,
    Sah,
  };

  class BvhPrimitive final: public Primitive {
    static constexpr uint32_t SAH_BUCKET_COUNT = 20;
    static constexpr int32_t SAH_INTERSECTION_COST = 2;
    static constexpr int32_t SAH_TRAVERSAL_COST = 1;

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

    struct LinearNode {
      Box bounds;
      uint32_t offset;
      uint8_t prim_count;
      uint8_t axis;
      uint8_t padding[2];
    };

    std::vector<LinearNode> linear_nodes;
    std::vector<std::unique_ptr<Primitive>> ordered_prims;
    uint32_t max_depth;
  public:
    BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
        uint32_t max_leaf_size, BvhSplitMethod split_method);
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const final override;
    virtual Box bounds() const override final;
  private:
    uint32_t build_node(
        std::vector<PrimitiveInfo>& build_infos,
        std::vector<std::unique_ptr<Primitive>>& prims,
        uint32_t begin, uint32_t end,
        uint32_t max_leaf_size, BvhSplitMethod split_method,
        uint32_t depth);
    uint32_t split_middle(std::vector<PrimitiveInfo>& build_infos,
        const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end);
    uint32_t split_equal_counts(std::vector<PrimitiveInfo>& build_infos,
        const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end);
    uint32_t split_sah(std::vector<PrimitiveInfo>& build_infos,
        const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end);
    int32_t sah_split_cost(const Box& bounds,
        const BucketInfo& bucket, uint32_t prim_count);

    template<class F>
    void traverse_primitives(const Ray& ray, F callback) const;

    static bool fast_box_hit_p(const Box& bounds, const Ray& ray,
        const Vector& inv_dir, bool dir_is_neg[3]);
  };
}

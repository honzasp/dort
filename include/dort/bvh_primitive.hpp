#pragma once
#include <memory>
#include "dort/primitive.hpp"

namespace dort {
  enum class BvhSplitMethod {
    Middle,
    EqualCounts,
  };

  class BvhPrimitive: public Primitive {
    struct PrimitiveInfo {
      uint32_t prim_index;
      Box bounds;
      Point centroid;
    };

    struct BuildNode {
      Box bounds;
      std::unique_ptr<BuildNode> children[2];
      uint32_t prims_begin;
      uint32_t prims_length;
      uint8_t axis;
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
    virtual Spectrum get_color(const DiffGeom& diff_geom) const override final;
    virtual float get_reflection(const DiffGeom& diff_geom) const override final;
  private:
    std::unique_ptr<BuildNode> build_node(
        std::vector<PrimitiveInfo>& build_infos,
        std::vector<std::unique_ptr<Primitive>>& prims,
        uint32_t begin, uint32_t end,
        uint32_t max_leaf_size, BvhSplitMethod split_method,
        uint32_t& out_node_count);
    uint32_t split_middle(std::vector<PrimitiveInfo>& build_infos,
        const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end);
    uint32_t split_equal_counts(std::vector<PrimitiveInfo>& build_infos,
        const Box& bounds, uint8_t axis, uint32_t begin, uint32_t end);
    void linearize_node(const BuildNode& node, uint32_t depth);

    template<class F>
    void traverse_primitives(const Ray& ray, F callback) const;

    static bool fast_box_hit_p(const Box& bounds, const Ray& ray,
        const Vector& inv_dir, bool dir_is_neg[3]);
  };
}

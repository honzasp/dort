#pragma once
#include "dort/bvh.hpp"
#include "dort/primitive.hpp"

namespace dort {
  class BvhPrimitive final: public Primitive {
    struct BvhTraits {
      using Element = std::unique_ptr<Primitive>;
      static Box get_bounds(const Element& elem) {
        return elem->bounds();
      }
    };

    Bvh<BvhTraits> bvh;
  public:
    BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
        const BvhOpts& opts, ThreadPool& pool);
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
  };
}

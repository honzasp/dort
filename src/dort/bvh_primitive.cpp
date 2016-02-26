#include "dort/bvh_primitive.hpp"
#include "dort/stats.hpp"

namespace dort {
  BvhPrimitive::BvhPrimitive(std::vector<std::unique_ptr<Primitive>> prims,
      const BvhOpts& opts, ThreadPool& pool):
    bvh(std::move(prims), opts, pool)
  { }

  bool BvhPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    stat_count(COUNTER_BVH_PRIM_INTERSECT);
    bool found = false;
    this->bvh.traverse_elems(ray, [&](const std::unique_ptr<Primitive>& prim) {
      if(prim->intersect(ray, out_isect)) {
        found = true;
      }
      return true;
    });
    if(found) {
      stat_count(COUNTER_BVH_PRIM_INTERSECT_HIT);
    }
    return found;
  }

  bool BvhPrimitive::intersect_p(const Ray& ray) const {
    stat_count(COUNTER_BVH_PRIM_INTERSECT_P);
    bool found = false;
    this->bvh.traverse_elems(ray, [&](const std::unique_ptr<Primitive>& prim) {
      if(prim->intersect_p(ray)) {
        found = true;
        return false;
      } else {
        return true;
      }
    });
    if(found) {
      stat_count(COUNTER_BVH_PRIM_INTERSECT_P_HIT);
    }
    return found;
  }

  Box BvhPrimitive::bounds() const {
    return this->bvh.bounds();
  }
}

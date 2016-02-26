#include "dort/list_primitive.hpp"

namespace dort {
  ListPrimitive::ListPrimitive(std::vector<std::unique_ptr<Primitive>> prims):
    prims(std::move(prims)),
    prim_bounds(this->prims.size())
  {
    for(uint32_t i = 0; i < this->prims.size(); ++i) {
      Box bounds = this->prims.at(i)->bounds();
      this->prim_bounds.at(i) = bounds;
      this->total_bounds = union_box(this->total_bounds, bounds);
    }
  }

  bool ListPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    Vector inv_dir(1.f / ray.dir.v.x, 1.f / ray.dir.v.y, 1.f / ray.dir.v.z);
    bool dir_is_neg[] = { ray.dir.v.x < 0.f, ray.dir.v.y < 0.f, ray.dir.v.z < 0.f };
    bool any_hit = false;

    for(uint32_t i = 0; i < this->prims.size(); ++i) {
      if(fast_box_hit_p(this->prim_bounds.at(i), ray, inv_dir, dir_is_neg)) {
        any_hit |= this->prims.at(i)->intersect(ray, out_isect);
      }
    }
    return any_hit;
  }

  bool ListPrimitive::intersect_p(const Ray& ray) const {
    Vector inv_dir(1.f / ray.dir.v.x, 1.f / ray.dir.v.y, 1.f / ray.dir.v.z);
    bool dir_is_neg[] = { ray.dir.v.x < 0.f, ray.dir.v.y < 0.f, ray.dir.v.z < 0.f };

    for(uint32_t i = 0; i < this->prims.size(); ++i) {
      if(fast_box_hit_p(this->prim_bounds.at(i), ray, inv_dir, dir_is_neg)) {
        if(this->prims.at(i)->intersect_p(ray)) {
          return true;
        }
      }
    }
    return false;
  }

  Box ListPrimitive::bounds() const {
    return this->total_bounds;
  }
}

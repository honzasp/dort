#include "dort/list_primitive.hpp"

namespace dort {
  ListPrimitive::ListPrimitive(std::vector<std::unique_ptr<Primitive>> prims):
    primitives(std::move(prims))
  {
    for(auto& prim: this->primitives) {
      this->total_bounds = union_box(this->total_bounds, prim->bounds());
    }
  }

  bool ListPrimitive::intersect(Ray& ray, Intersection& out_isect) const
  {
    bool any_hit = false;
    for(auto& prim: this->primitives) {
      any_hit |= prim->intersect(ray, out_isect);
    }
    return any_hit;
  }

  Box ListPrimitive::bounds() const
  {
    return this->total_bounds;
  }

  Spectrum ListPrimitive::get_color(const DiffGeom&) const
  {
    assert("ListPrimitive::get_color called");
    return Spectrum();
  }
}

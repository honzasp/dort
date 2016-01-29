#pragma once
#include <memory>
#include "dort/primitive.hpp"
#include "dort/transform.hpp"

namespace dort {
  class TransformPrimitive: public Primitive {
    Transform obj_to_world;
    std::unique_ptr<Primitive> inside;
  public:
    TransformPrimitive(const Transform& obj_to_world,
        std::unique_ptr<Primitive> inside):
      obj_to_world(obj_to_world), inside(std::move(inside)) { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const final override;
    virtual Box bounds() const final override;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const final override;
  };
}

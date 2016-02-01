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

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual Spectrum get_color(const DiffGeom& diff_geom) const override final;
    virtual float get_reflection(const DiffGeom& diff_geom) const override final;
  };
}

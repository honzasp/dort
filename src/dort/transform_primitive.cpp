#include "dort/transform_primitive.hpp"

namespace dort {
  bool TransformPrimitive::intersect(Ray& ray, Intersection& out_isect) const
  {
    Ray new_ray(this->obj_to_world.apply_inv(ray));
    if(!this->inside->intersect(new_ray, out_isect)) {
      return false;
    }
    ray.t_max = new_ray.t_max;
    out_isect.diff_geom = this->obj_to_world.apply(out_isect.diff_geom);
    return true;
  }

  bool TransformPrimitive::intersect_p(const Ray& ray) const
  {
    Ray new_ray(this->obj_to_world.apply_inv(ray));
    return this->inside->intersect_p(new_ray);
  }

  Box TransformPrimitive::bounds() const
  {
    return this->obj_to_world.apply(this->inside->bounds());
  }

  Spectrum TransformPrimitive::get_color(const DiffGeom&) const
  {
    assert("TransformPrimitive::get_color called");
    return Spectrum();
  }

  float TransformPrimitive::get_reflection(const DiffGeom&) const
  {
    assert("TransformPrimitive::get_reflection called");
    return 0.f;
  }
}

#include "dort/geometric_primitive.hpp"

namespace dort {
  GeometricPrimitive::GeometricPrimitive(
      std::shared_ptr<Shape> shape, Spectrum color, float reflection):
    shape(std::move(shape)),
    color(color),
    reflection(reflection)
  { }

  bool GeometricPrimitive::intersect(Ray& ray, Intersection& out_isect) const
  {
    float hit_t;
    if(!this->shape->hit(ray, hit_t, out_isect.ray_epsilon,
          out_isect.diff_geom)) {
      return false;
    }

    assert(hit_t <= ray.t_max);
    ray.t_max = hit_t;
    out_isect.primitive = this;
    return true;
  }

  bool GeometricPrimitive::intersect_p(const Ray& ray) const
  {
    return this->shape->hit_p(ray);
  }

  Box GeometricPrimitive::bounds() const
  {
    return this->shape->bound();
  }

  Spectrum GeometricPrimitive::get_color(const DiffGeom&) const
  {
    return this->color;
  }

  float GeometricPrimitive::get_reflection(const DiffGeom&) const
  {
    return this->reflection;
  }
}

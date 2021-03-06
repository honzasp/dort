#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/triangle.hpp"
#include "dort/triangle_shape_primitive.hpp"

namespace dort {
  bool TriangleShapePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    float t_hit;
    TriangleUv triangle(*this->mesh, this->index);
    if(!triangle.hit(ray, t_hit, out_isect.ray_epsilon, out_isect.frame_diff_geom)) {
      return false;
    }
    out_isect.world_diff_geom = out_isect.frame_diff_geom;
    out_isect.primitive = this;
    ray.t_max = t_hit;
    return true;
  }

  bool TriangleShapePrimitive::intersect_p(const Ray& ray) const {
    return Triangle(*this->mesh, this->index).hit_p(ray);
  }

  Box TriangleShapePrimitive::bounds() const {
    return Triangle(*this->mesh, this->index).bounds();
  }

  const Material* TriangleShapePrimitive::get_material(const Intersection&) const {
    return this->material.get();
  }

  const Light* TriangleShapePrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }
}

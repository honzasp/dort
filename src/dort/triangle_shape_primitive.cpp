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

  std::unique_ptr<Bsdf> TriangleShapePrimitive::get_bsdf(const Intersection& isect) const {
    return this->material->get_bsdf(isect.frame_diff_geom);
  }

  const AreaLight* TriangleShapePrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }
}

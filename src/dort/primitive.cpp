#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/primitive.hpp"

namespace dort {
  GeometricPrimitive::GeometricPrimitive(
      std::shared_ptr<Shape> shape,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light):
    shape(std::move(shape)),
    material(std::move(material)),
    area_light(std::move(area_light))
  { }

  bool GeometricPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
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

  bool GeometricPrimitive::intersect_p(const Ray& ray) const {
    return this->shape->hit_p(ray);
  }

  Box GeometricPrimitive::bounds() const {
    return this->shape->bound();
  }

  std::unique_ptr<Bsdf> GeometricPrimitive::get_bsdf(const DiffGeom& diff_geom) const {
    return this->material->get_bsdf(diff_geom);
  }

  const AreaLight* GeometricPrimitive::get_area_light(const DiffGeom&) const {
    return this->area_light.get();
  }

  bool TransformPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    Ray new_ray(this->prim_to_world.apply_inv(ray));
    if(!this->inside->intersect(new_ray, out_isect)) {
      return false;
    }
    ray.t_max = new_ray.t_max;
    out_isect.diff_geom = this->prim_to_world.apply(out_isect.diff_geom);
    return true;
  }

  bool TransformPrimitive::intersect_p(const Ray& ray) const {
    Ray new_ray(this->prim_to_world.apply_inv(ray));
    return this->inside->intersect_p(new_ray);
  }

  Box TransformPrimitive::bounds() const {
    return this->prim_to_world.apply(this->inside->bounds());
  }
}

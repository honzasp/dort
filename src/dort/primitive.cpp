#include "dort/bsdf.hpp"
#include "dort/light.hpp"
#include "dort/material.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum Intersection::emitted_radiance(const Vector& wo) const {
    const AreaLight* area_light = this->primitive->get_area_light(
        this->frame_diff_geom);
    if(area_light) {
      return area_light->emitted_radiance(this->world_diff_geom.p,
          this->world_diff_geom.nn, wo);
    } else {
      return Spectrum(0.f);
    }
  }

  bool ShapePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    Ray new_ray(this->shape_to_frame.apply_inv(ray));
    float t_hit;
    if(!this->shape->hit(new_ray, t_hit, out_isect.ray_epsilon,
        out_isect.frame_diff_geom)) {
      return false;
    }
    out_isect.world_diff_geom = out_isect.frame_diff_geom =
      this->shape_to_frame.apply(out_isect.frame_diff_geom);
    out_isect.primitive = this;
    ray.t_max = t_hit;
    return true;
  }

  bool ShapePrimitive::intersect_p(const Ray& ray) const {
    Ray new_ray(this->shape_to_frame.apply_inv(ray));
    return this->shape->hit_p(new_ray);
  }

  Box ShapePrimitive::bounds() const {
    return this->shape_to_frame.apply(this->shape->bounds());
  }

  std::unique_ptr<Bsdf> ShapePrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const 
  {
    return this->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* ShapePrimitive::get_area_light(const DiffGeom&) const {
    return this->area_light.get();
  }


  bool FramePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    Ray new_ray(this->in_to_out.apply_inv(ray));
    if(!this->inside->intersect(new_ray, out_isect)) {
      return false;
    }
    ray.t_max = new_ray.t_max;
    out_isect.world_diff_geom = this->in_to_out.apply(out_isect.world_diff_geom);
    return true;
  }

  bool FramePrimitive::intersect_p(const Ray& ray) const {
    Ray new_ray(this->in_to_out.apply_inv(ray));
    return this->inside->intersect_p(new_ray);
  }

  Box FramePrimitive::bounds() const {
    return this->in_to_out.apply(this->inside->bounds());
  }
}

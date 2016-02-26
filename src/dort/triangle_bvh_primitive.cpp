#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/triangle_bvh_primitive.hpp"

namespace dort {
  TriangleBvhPrimitive::TriangleBvhPrimitive(const TriangleMesh* mesh,
      std::vector<uint32_t> indices, const BvhOpts& opts, ThreadPool& pool):
    mesh(mesh),
    bvh(std::move(indices), mesh, opts, pool)
  { }

  bool TriangleBvhPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    bool found = false;
    this->bvh.traverse_elems(ray, [&](uint32_t index) {
      float t_hit;
      if(Triangle(this->mesh, index).hit(ray, t_hit, out_isect.ray_epsilon,
          out_isect.frame_diff_geom)) 
      {
        found = true;
        ray.t_max = t_hit;
      }
      return true;
    });

    if(found) {
      out_isect.world_diff_geom = out_isect.frame_diff_geom;
      out_isect.primitive = this;
    }

    return found;
  }

  bool TriangleBvhPrimitive::intersect_p(const Ray& ray) const {
    bool found = false;
    this->bvh.traverse_elems(ray, [&](uint32_t index) {
      if(Triangle(this->mesh, index).hit_p(ray)) {
        found = true;
        return false;
      } else {
        return true;
      }
    });
    return found;
  }

  Box TriangleBvhPrimitive::bounds() const {
    return this->bvh.bounds();
  }

  std::unique_ptr<Bsdf> TriangleBvhPrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const
  {
    return this->mesh->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* TriangleBvhPrimitive::get_area_light(const DiffGeom&) const {
    return this->mesh->area_light.get();
  }
}

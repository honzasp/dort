#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/mesh.hpp"
#include "dort/mesh_triangle_primitive.hpp"
#include "dort/triangle.hpp"

namespace dort {
  bool MeshTrianglePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    float t_hit;
    TriangleUv triangle(this->prim_mesh->mesh, this->index);
    if(!triangle.hit(ray, t_hit, out_isect.ray_epsilon, out_isect.frame_diff_geom)) {
      return false;
    }
    out_isect.world_diff_geom = out_isect.frame_diff_geom;
    out_isect.primitive = this;
    ray.t_max = t_hit;
    return true;
  }

  bool MeshTrianglePrimitive::intersect_p(const Ray& ray) const {
    return Triangle(this->prim_mesh->mesh, this->index).hit_p(ray);
  }

  Box MeshTrianglePrimitive::bounds() const {
    return Triangle(this->prim_mesh->mesh, this->index).bounds();
  }

  std::unique_ptr<Bsdf> MeshTrianglePrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const 
  {
    return this->prim_mesh->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* MeshTrianglePrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }
}

#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/mesh_bvh_primitive.hpp"

namespace dort {
  MeshBvhPrimitive::MeshBvhPrimitive(const Mesh* mesh,
      std::shared_ptr<Material> material, std::vector<uint32_t> indices,
      const BvhOpts& opts, ThreadPool& pool):
    mesh(mesh), material(material),
    bvh(std::move(indices), mesh, opts, pool)
  { }

  bool MeshBvhPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    bool found = false;
    this->bvh.traverse_elems(ray, [&](uint32_t index) {
      float t_hit;
      if(TriangleUv(*this->mesh, index).hit(ray, t_hit, out_isect.ray_epsilon,
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

  bool MeshBvhPrimitive::intersect_p(const Ray& ray) const {
    bool found = false;
    this->bvh.traverse_elems(ray, [&](uint32_t index) {
      if(Triangle(*this->mesh, index).hit_p(ray)) {
        found = true;
        return false;
      } else {
        return true;
      }
    });
    return found;
  }

  Box MeshBvhPrimitive::bounds() const {
    return this->bvh.bounds();
  }

  std::unique_ptr<Bsdf> MeshBvhPrimitive::get_bsdf(const Intersection& isect) const {
    return this->material->get_bsdf(isect.world_diff_geom);
  }

  const AreaLight* MeshBvhPrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }
}

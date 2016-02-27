#pragma once
#include "dort/bvh.hpp"
#include "dort/mesh.hpp"
#include "dort/primitive.hpp"
#include "dort/triangle.hpp"

namespace dort {
  class MeshBvhPrimitive final: public GeometricPrimitive {
    struct BvhTraits {
      using Element = uint32_t;
      using Arg = const Mesh*;

      static Box get_bounds(const Mesh* mesh, uint32_t idx) {
        return Triangle(*mesh, idx).bounds();
      }
    };

    const Mesh* mesh;
    std::shared_ptr<Material> material;
    Bvh<BvhTraits> bvh;
  public:
    MeshBvhPrimitive(const Mesh* mesh, std::shared_ptr<Material> material,
        std::vector<uint32_t> indices, const BvhOpts& opts, ThreadPool& pool);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };
}

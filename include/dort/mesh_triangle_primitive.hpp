#pragma once
#include "dort/primitive.hpp"

namespace dort {
  class MeshTrianglePrimitive final: public GeometricPrimitive {
    const PrimitiveMesh* prim_mesh;
    uint32_t index;
  public:
    MeshTrianglePrimitive(const PrimitiveMesh* prim_mesh, uint32_t index):
      prim_mesh(prim_mesh), index(index) { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };
}

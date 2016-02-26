#pragma once
#include "dort/bvh.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  class TriangleBvhPrimitive final: public GeometricPrimitive {
    const TriangleMesh* mesh;
    Bvh<> bvh;
  public:
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  private:
    Triangle get_triangle() const;
  };
}

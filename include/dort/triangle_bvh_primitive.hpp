#pragma once
#include "dort/bvh.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  class TriangleBvhPrimitive final: public GeometricPrimitive {
    struct BvhTraits {
      using Element = uint32_t;
      using Arg = const TriangleMesh*;

      static Box get_bounds(const TriangleMesh* mesh, uint32_t idx) {
        return Triangle(mesh, idx).bounds();
      }
    };

    const TriangleMesh* mesh;
    Bvh<BvhTraits> bvh;
  public:
    TriangleBvhPrimitive(const TriangleMesh* mesh, std::vector<uint32_t> indices,
        const BvhOpts& opts, ThreadPool& pool);

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

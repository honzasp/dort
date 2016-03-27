#pragma once
#include "dort/primitive.hpp"

namespace dort {
  class TriangleShapePrimitive final: public GeometricPrimitive {
    const Mesh* mesh;
    uint32_t index;
    std::shared_ptr<Material> material;
  public:
    TriangleShapePrimitive(const Mesh* mesh, uint32_t index,
        std::shared_ptr<Material> material):
      mesh(mesh), index(index), material(material) { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const Intersection& isect) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };
}

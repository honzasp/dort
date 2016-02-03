#pragma once
#include "dort/shape.hpp"
#include "dort/spectrum.hpp"
#include "dort/transform.hpp"

namespace dort {
  struct Intersection {
    float ray_epsilon;
    DiffGeom diff_geom;
    const GeometricPrimitive* primitive;
  };

  class Primitive {
  public:
    Primitive() { };
    virtual ~Primitive() { };
    virtual bool intersect(Ray& ray, Intersection& out_isect) const = 0;
    virtual bool intersect_p(const Ray& ray) const = 0;
    virtual Box bounds() const = 0;
  };

  class GeometricPrimitive: public Primitive {
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> area_light;
  public:
    GeometricPrimitive(std::shared_ptr<Shape> shape,
        std::shared_ptr<Material> material,
        std::shared_ptr<AreaLight> area_light = nullptr);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const final override;
    virtual Box bounds() const override final;

    std::unique_ptr<Bsdf> get_bsdf(const DiffGeom& diff_geom) const;
    const AreaLight* get_area_light(const DiffGeom& diff_geom) const;
  };

  class TransformPrimitive: public Primitive {
    Transform prim_to_world;
    std::unique_ptr<Primitive> inside;
  public:
    TransformPrimitive(const Transform& prim_to_world,
        std::unique_ptr<Primitive> inside):
      prim_to_world(prim_to_world), inside(std::move(inside)) { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
  };
}

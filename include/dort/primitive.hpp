#pragma once
#include "dort/shape.hpp"
#include "dort/spectrum.hpp"
#include "dort/transform.hpp"

namespace dort {
  struct Intersection {
    float ray_epsilon;
    DiffGeom frame_diff_geom;
    DiffGeom world_diff_geom;
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
  public:
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const = 0;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const = 0;
  };

  class ShapePrimitive: public GeometricPrimitive {
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> area_light;
    Transform shape_to_frame;
  public:
    ShapePrimitive(std::shared_ptr<Shape> shape,
        std::shared_ptr<Material> material,
        std::shared_ptr<AreaLight> area_light,
        const Transform& shape_to_frame):
      shape(shape), material(material),
      area_light(area_light), shape_to_frame(shape_to_frame)
    { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };

  class FramePrimitive: public Primitive {
    Transform in_to_out;
    std::shared_ptr<Primitive> inside;
  public:
    FramePrimitive(const Transform& in_to_out,
        std::shared_ptr<Primitive> inside):
      in_to_out(in_to_out), inside(std::move(inside)) { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
  };
}

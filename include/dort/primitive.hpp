#pragma once
#include "dort/box.hpp"
#include "dort/shape.hpp"
#include "dort/spectrum.hpp"
#include "dort/transform.hpp"

namespace dort {
  struct Intersection {
    float ray_epsilon;
    // TODO: rename to something shorter
    DiffGeom frame_diff_geom;
    DiffGeom world_diff_geom;
    const GeometricPrimitive* primitive;

    union {
      uint32_t aux_uint32[4];
      int32_t aux_int32[4];
      float aux_float[4];
    };

    std::unique_ptr<Bsdf> get_bsdf() const;
    const Light* get_area_light() const;
    Spectrum eval_radiance(const Point& pivot) const;
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
    virtual const Material* get_material(
        const Intersection& isect) const = 0;
    virtual const Light* get_area_light(
        const DiffGeom& frame_diff_geom) const = 0;
  };

  class ShapePrimitive final: public GeometricPrimitive {
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;
    std::shared_ptr<Light> area_light;
    Transform shape_to_frame;
  public:
    ShapePrimitive(std::shared_ptr<Shape> shape,
        std::shared_ptr<Material> material,
        std::shared_ptr<Light> area_light,
        const Transform& shape_to_frame):
      shape(shape), material(material),
      area_light(area_light), shape_to_frame(shape_to_frame)
    { }

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual const Material* get_material(
        const Intersection& isect) const override final;
    virtual const Light* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };

  class FramePrimitive final: public Primitive {
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

#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/primitive.hpp"
#include "dort/shape.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  struct TriangleMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> area_light;
  };

  struct Triangle final {
    Point p[3];
    Vec2 uv[2];

    Triangle(const TriangleMesh* mesh, uint32_t index);

    bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const;
    bool hit_p(const Ray& ray) const;
    Box bounds() const;

    float area() const;
    Point sample_point(float u1, float u2, Normal& out_n) const;
    float point_pdf(const Point& pt) const;
  };

  class TriangleShape final: public Shape {
    const TriangleMesh* mesh;
    uint32_t index;
  public:
    TriangleShape(const TriangleMesh* mesh, uint32_t index):
      mesh(mesh), index(index) { }
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual float area() const override final;
    virtual Point sample_point(float u1, float u2, Normal& out_n) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  private:
    Triangle get_triangle() const;
  };

  class TrianglePrimitive final: public GeometricPrimitive {
    const TriangleMesh* mesh;
    uint32_t index;
  public:
    TrianglePrimitive(const TriangleMesh* mesh, uint32_t index):
      mesh(mesh), index(index) { }
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

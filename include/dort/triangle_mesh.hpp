#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/primitive.hpp"
#include "dort/shape.hpp"

namespace dort {
  struct TriangleMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
    std::shared_ptr<Material> material;
    std::shared_ptr<AreaLight> area_light;
  };

  class Triangle: public Shape {
    const TriangleMesh* mesh;
    uint32_t index;
  public:
    Triangle(const TriangleMesh* mesh, uint32_t id):
      mesh(mesh), index(id * 3) { }
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual float area() const override final;
    virtual Point sample_point(float u1, float u2, Normal& out_n) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  private:
    void get_points(Point p[3]) const;
    void get_uvs(float uv[3][2]) const;

    friend class TrianglePrimitive;
  };

  class TrianglePrimitive: public GeometricPrimitive {
    Triangle triangle;
  public:
    TrianglePrimitive(const Triangle& triangle):
      triangle(triangle) { }
    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  };
}

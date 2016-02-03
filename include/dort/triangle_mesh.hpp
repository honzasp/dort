#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/shape.hpp"

namespace dort {
  struct TriangleMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
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
    virtual Box bound() const override final;
  private:
    void get_points(Point p[3]) const;
    void get_uvs(float uv[3][2]) const;
  };
}

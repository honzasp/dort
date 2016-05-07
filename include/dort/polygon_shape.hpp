#pragma once
#include <vector>
#include "dort/rect.hpp"
#include "dort/shape.hpp"

namespace dort {
  class PolygonShape final: public Shape {
    std::vector<Vec2> vertices;
    Rect rect_bounds;
  public:
    PolygonShape(std::vector<Vec2> vertices);

    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const override final;
    virtual bool hit_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;

    virtual float area() const override final;
    virtual Point sample_point(float u1, float u2,
        Normal& out_n, float& ray_epsilon) const override final;
    virtual float point_pdf(const Point& pt) const override final;
  private:
    bool ray_hit_plane(const Ray& ray, Vec2& out_p_hit, float& out_t_hit) const;
    bool point_in_polygon(Vec2 pt) const;
  };
}

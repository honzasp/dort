#include "dort/polygon_shape.hpp"
#include "dort/rng.hpp"

namespace dort {
  PolygonShape::PolygonShape(std::vector<Vec2> vertices):
    vertices(std::move(vertices))
  {
    this->rect_bounds = Rect();
    for(auto vertex: this->vertices) {
      this->rect_bounds = union_rect(this->rect_bounds, vertex);
    }
    this->poly_area = this->compute_area();
  }

  bool PolygonShape::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    Vec2 p_hit;
    float t_hit;
    if(!this->ray_hit_plane(ray, p_hit, t_hit)) {
      return false;
    }

    if(!this->point_in_polygon(p_hit)) {
      return false;
    }

    out_t_hit = t_hit;
    out_ray_epsilon = 1e-3f;
    out_diff_geom.p = Point(p_hit.x, p_hit.y, 0.f);
    out_diff_geom.nn = Normal(0.f, 0.f, 1.f);
    out_diff_geom.u = p_hit.x;
    out_diff_geom.v = p_hit.y;
    out_diff_geom.dpdu = Vector(1.f, 0.f, 0.f);
    out_diff_geom.dpdv = Vector(0.f, 1.f, 0.f);
    return true;
  }

  bool PolygonShape::hit_p(const Ray& ray) const {
    Vec2 p_hit; float t_hit;
    return this->ray_hit_plane(ray, p_hit, t_hit) && this->point_in_polygon(p_hit);
  }

  Box PolygonShape::bounds() const {
    return Box(
        Point(this->rect_bounds.p_min.x, this->rect_bounds.p_min.y, 0.f),
        Point(this->rect_bounds.p_max.x, this->rect_bounds.p_max.y, 0.f));
  }

  float PolygonShape::area() const {
    return this->poly_area;
  }

  Point PolygonShape::sample_point(Vec2 uv, float& out_pos_pdf,
      Normal& out_n, float& out_ray_epsilon) const
  {
    Rng rng(floor_uint32(uv.x * 10729 + uv.y * 23801));

    for(uint32_t i = 0; i < 100; ++i) {
      Vec2 point = this->rect_bounds.sample(uv);
      if(!this->point_in_polygon(point)) {
        uv = Vec2(rng.uniform_float(), rng.uniform_float());
        continue;
      }
      out_pos_pdf = 1.f / this->area();
      out_n = Normal(0.f, 0.f, 1.f);
      out_ray_epsilon = 1e-3f;
      return Point(point.x, point.y, 0.f);
    }

    out_pos_pdf = 0.f;
    out_n = Normal(0.f, 0.f, 1.f);
    out_ray_epsilon = 1e-3f;
    return Point();
  }

  float PolygonShape::point_pdf(const Point&) const {
    return 1.f / this->area();
  }

  bool PolygonShape::ray_hit_plane(const Ray& ray,
    Vec2& out_p_hit, float& out_t_hit) const
  {
    out_t_hit = -ray.orig.v.z / ray.dir.v.z;
    if(!(out_t_hit >= ray.t_min && out_t_hit <= ray.t_max)) {
      return false;
    }

    out_p_hit = Vec2(ray.orig.v.x + ray.dir.v.x * out_t_hit,
        ray.orig.v.y + ray.dir.v.y * out_t_hit);
    return this->rect_bounds.contains(out_p_hit);
  }

  bool PolygonShape::point_in_polygon(Vec2 pt) const {
    uint32_t isects = 0;
    Vec2 v0 = this->vertices.at(this->vertices.size() - 1) - pt;
    for(uint32_t edge = 0; edge < this->vertices.size(); ++edge) {
      Vec2 v1 = this->vertices.at(edge) - pt;
      if(v0.y * v1.y < 0.f) {
        bool pos0 = v0.x > 0.f;
        bool pos1 = v1.x > 0.f;
        if(pos0 && pos1) {
          ++isects;
        } else if(pos0 || pos1) {
          if(v1.y * (v1.x - v0.x) * (v1.y - v0.y) <= v1.x * square(v1.y - v0.y)) {
            ++isects;
          }
        }
      }
      v0 = v1;
    }
    return isects % 2 == 1;
  }

  float PolygonShape::compute_area() const {
    float double_area = 0.f;
    uint32_t n = this->vertices.size();
    for(uint32_t i = 0; i < n; ++i) {
      float x_i = this->vertices.at(i).x;
      float y_ip = this->vertices.at(i == n - 1 ? 0 : i + 1).y;
      float y_im = this->vertices.at(i == 0 ? n - 1 : i - 1).y;
      double_area += x_i * (y_ip - y_im);
    }
    return 0.5f * abs(double_area);
  }
}


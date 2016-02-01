#include "dort/triangle_mesh.hpp"

namespace dort {
  bool Triangle::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    Point p0, p1, p2;
    this->get_points(p0, p1, p2);

    Vector e1 = p1 - p0;
    Vector e2 = p2 - p0;
    Vector s1 = cross(ray.dir, e2);
    float det = dot(s1, e1);
    if(det == 0.f) {
      return false;
    }
    float inv_det = 1.f / det;

    Vector s = ray.orig - p0;
    float b1 = inv_det * dot(s1, s);
    if(b1 < 0.f || b1 > 1.f) {
      return false;
    }

    Vector s2 = cross(s, e1);
    float b2 = inv_det * dot(s2, ray.dir);
    if(b2 < 0.f || b1 + b2 > 1.f) {
      return false;
    }

    float t = inv_det * dot(s2, e2);
    if(t < ray.t_min || t > ray.t_max) {
      return false;
    }

    out_t_hit = t;
    out_ray_epsilon = abs(1e-3f * t);
    out_diff_geom.p = (1.f - b1 - b2) * p0 + b1 * p1 + b2 * p2;
    out_diff_geom.nn = normalize(Normal(cross(e1, e2)));
    return true;
  }

  bool Triangle::hit_p(const Ray& ray) const
  {
    Point p0, p1, p2;
    this->get_points(p0, p1, p2);

    Vector e1 = p1 - p0;
    Vector e2 = p2 - p0;
    Vector s1 = cross(ray.dir, e2);
    float det = dot(s1, e1);
    if(det == 0.f) {
      return false;
    }
    float inv_det = 1.f / det;

    Vector s = ray.orig - p0;
    float b1 = inv_det * dot(s1, s);
    if(b1 < 0.f || b1 > 1.f) {
      return false;
    }

    Vector s2 = cross(s, e1);
    float b2 = inv_det * dot(s2, ray.dir);
    if(b2 < 0.f || b1 + b2 > 1.f) {
      return false;
    }

    float t = inv_det * dot(s2, e2);
    if(t < ray.t_min || t > ray.t_max) {
      return false;
    }
    return true;
  }

  Box Triangle::bound() const
  {
    Point p0, p1, p2;
    this->get_points(p0, p1, p2);

    Box bound;
    bound = union_box(bound, p0);
    bound = union_box(bound, p1);
    bound = union_box(bound, p2);
    return bound;
  }

  void Triangle::get_points(Point& p0, Point& p1, Point& p2) const
  {
    auto& pts = this->mesh->points;
    auto& verts = this->mesh->vertices;
    p0 = pts.at(verts.at(this->index));
    p1 = pts.at(verts.at(this->index + 1));
    p2 = pts.at(verts.at(this->index + 2));
  }
}

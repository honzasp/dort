#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/stats.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  bool TriangleShape::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    return this->get_triangle().hit(ray, out_t_hit, out_ray_epsilon, out_diff_geom);
  }

  bool TriangleShape::hit_p(const Ray& ray) const {
    return this->get_triangle().hit_p(ray);
  }

  Box TriangleShape::bounds() const {
    return this->get_triangle().bounds();
  }

  float TriangleShape::area() const {
    return this->get_triangle().area();
  }

  Point TriangleShape::sample_point(float u1, float u2, Normal& out_n) const {
    return this->get_triangle().sample_point(u1, u2, out_n);
  }

  float TriangleShape::point_pdf(const Point& pt) const {
    return this->get_triangle().point_pdf(pt);
  }

  Triangle TriangleShape::get_triangle() const {
    return Triangle(this->mesh, this->index);
  }

  bool TrianglePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    float t_hit;
    if(!this->get_triangle().hit(ray, t_hit, out_isect.ray_epsilon,
          out_isect.frame_diff_geom)) {
      return false;
    }
    out_isect.world_diff_geom = out_isect.frame_diff_geom;
    out_isect.primitive = this;
    ray.t_max = t_hit;
    return true;
  }

  bool TrianglePrimitive::intersect_p(const Ray& ray) const {
    return this->get_triangle().hit_p(ray);
  }

  Box TrianglePrimitive::bounds() const {
    return this->get_triangle().bounds();
  }

  std::unique_ptr<Bsdf> TrianglePrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const 
  {
    return this->mesh->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* TrianglePrimitive::get_area_light(const DiffGeom&) const {
    return this->mesh->area_light.get();
  }

  Triangle TrianglePrimitive::get_triangle() const {
    return Triangle(this->mesh, this->index);
  }
}

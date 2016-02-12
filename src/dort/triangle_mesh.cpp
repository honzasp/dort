#include "dort/bsdf.hpp"
#include "dort/triangle_mesh.hpp"
#include "dort/material.hpp"

namespace dort {
  bool Triangle::hit(const Ray& ray, float& out_t_hit,
      float& out_ray_epsilon, DiffGeom& out_diff_geom) const
  {
    Point p[3];
    this->get_points(p);

    Vector e1 = p[1] - p[0];
    Vector e2 = p[2] - p[0];
    Vector s1 = cross(ray.dir, e2);
    float det = dot(s1, e1);
    if(det == 0.f) {
      return false;
    }
    float inv_det = 1.f / det;

    Vector s = ray.orig - p[0];
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

    float uv[3][2];
    this->get_uvs(uv);
    float du1 = uv[1][0] - uv[0][0];
    float du2 = uv[2][0] - uv[0][0];
    float dv1 = uv[1][1] - uv[0][1];
    float dv2 = uv[2][1] - uv[0][1];
    float uv_deter = du1 * dv2 - dv1 * du2;
    if(uv_deter == 0.f) {
      coordinate_system(cross(e2, e1), out_diff_geom.dpdu, out_diff_geom.dpdv);
    } else {
      float inv_uv_deter = 1.f / uv_deter;
      out_diff_geom.dpdu = (e1 * dv2 - e2 * dv1) * inv_uv_deter;
      out_diff_geom.dpdv = (e2 * du1 - e1 * du2) * inv_uv_deter;
    }

    float b0 = 1.f - b1 - b2;
    float u = b0 * uv[0][0] + b1 * uv[1][0] + b2 * uv[2][0];
    float v = b0 * uv[0][1] + b1 * uv[1][1] + b2 * uv[2][1];

    out_t_hit = t;
    out_ray_epsilon = abs(1e-3f * t);
    out_diff_geom.p = b0 * p[0] + b1 * p[1] + b2 * p[2];
    out_diff_geom.nn = normalize(Normal(cross(e1, e2)));
    out_diff_geom.u = u;
    out_diff_geom.v = v;
    return true;
  }

  bool Triangle::hit_p(const Ray& ray) const {
    Point p[3];
    this->get_points(p);

    Vector e1 = p[1] - p[0];
    Vector e2 = p[2] - p[0];
    Vector s1 = cross(ray.dir, e2);
    float det = dot(s1, e1);
    if(det == 0.f) {
      return false;
    }
    float inv_det = 1.f / det;

    Vector s = ray.orig - p[0];
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

  Box Triangle::bounds() const {
    Point p[3];
    this->get_points(p);

    Box bound;
    bound = union_box(bound, p[0]);
    bound = union_box(bound, p[1]);
    bound = union_box(bound, p[2]);
    return bound;
  }

  float Triangle::area() const {
    Point p[3];
    this->get_points(p);
    Vector e1 = p[1] - p[0];
    Vector e2 = p[2] - p[0];
    return 0.5f * length(cross(e1, e2));
  }

  Point Triangle::sample_point(float u1, float u2, Normal& out_n) const {
    Point p[3];
    this->get_points(p);
    Vector e1 = p[1] - p[0];
    Vector e2 = p[2] - p[0];

    float s = sqrt(u1);
    float b1 = 1.f - s;
    float b2 = u2 * s;

    out_n = Normal(normalize(cross(e2, e1)));
    return p[0] + b1 * e1 + b2 * e2;
  }

  float Triangle::point_pdf(const Point&) const {
    return 1.f / this->area();
  }

  void Triangle::get_points(Point p[3]) const {
    auto& pts = this->mesh->points;
    auto& verts = this->mesh->vertices;
    p[0] = pts.at(verts.at(this->index));
    p[1] = pts.at(verts.at(this->index + 1));
    p[2] = pts.at(verts.at(this->index + 2));
  }

  void Triangle::get_uvs(float uv[3][2]) const {
    uv[0][0] = 0.f;
    uv[0][1] = 0.f;
    uv[1][0] = 1.f;
    uv[1][1] = 0.f;
    uv[2][0] = 0.f;
    uv[2][1] = 1.f;
  }

  bool TrianglePrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    float t_hit;
    if(!this->triangle.hit(ray, t_hit, out_isect.ray_epsilon,
          out_isect.frame_diff_geom)) {
      return false;
    }
    out_isect.world_diff_geom = out_isect.frame_diff_geom;
    out_isect.primitive = this;
    ray.t_max = t_hit;
    return true;
  }

  bool TrianglePrimitive::intersect_p(const Ray& ray) const {
    return this->triangle.hit_p(ray);
  }

  Box TrianglePrimitive::bounds() const {
    return this->triangle.bounds();
  }

  std::unique_ptr<Bsdf> TrianglePrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const 
  {
    return this->triangle.mesh->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* TrianglePrimitive::get_area_light(const DiffGeom&) const {
    return this->triangle.mesh->area_light.get();
  }
}

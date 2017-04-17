#pragma once
#include "dort/box.hpp"
#include "dort/geometry.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  struct DiffGeom {
    Point p;
    Vec2 uv;
    Normal nn;
    Vector dpdu;
    Vector dpdv;
    Normal nn_shading;
    Vector dpdu_shading;
    Vector dpdv_shading;
  };

  class Shape {
  public:
    Shape() {}
    virtual bool hit(const Ray& ray, float& out_t_hit,
        float& out_ray_epsilon, DiffGeom& out_diff_geom) const = 0;
    virtual bool hit_p(const Ray& ray) const = 0;
    virtual Box bounds() const = 0;
    virtual float area() const = 0;

    /// Samples a point on the shape using the uv as a random sample.
    virtual Point sample_point(Vec2 uv,
        float& out_pos_pdf, Normal& out_n, float& out_ray_epsilon) const = 0;

    /// Samples a point on the shape visible from pivot, using the uv as a
    /// random sample.
    /// Outputs the directional pdfs as in point_pivot_pdf() by out_dir_pdf.
    virtual Point sample_point_pivot(const Point& pivot, Vec2 uv,
        float& out_dir_pdf, Normal& out_n, float& out_ray_epsilon) const;

    /// Computes the pdf of sampling the point pt (which is assumed to lie on
    /// the shape modulo floating-point error) with sample_point(), w.r.t. the
    /// area measure.
    virtual float point_pdf(const Point& pt) const = 0;

    /// Computes the pdf of sampling direction w from the shape using
    /// sample_point_pivot() with pivot, w.r.t. solid angle at the shape point.
    ///
    /// @warning This method takes the direction from the shape, while
    /// sample_point_pivot() returns directly the point on the shape. The pdf is
    /// with respect to solid angle at the sampled point.
    virtual float point_pivot_pdf(const Point& pivot, const Vector& w) const;
  };
}

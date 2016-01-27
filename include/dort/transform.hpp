#pragma once
#include "dort/geometry.hpp"
#include "dort/mat.hpp"
#include "dort/shape.hpp"

namespace dort {
  class Transform {
    Mat4x4 mat;
    Mat4x4 mat_inv;
  public:
    explicit Transform(const Mat4x4& mat, const Mat4x4& mat_inv):
      mat(mat), mat_inv(mat_inv) { }

    Transform inverse() const;
    Transform operator*(const Transform& trans) const;

    Vector apply(const Vector& vec) const;
    Point apply(const Point& pt) const;
    Normal apply(const Normal& pt) const;
    DiffGeom apply(const DiffGeom& dg) const;
    Box apply(const Box& box) const;
    Ray apply(const Ray& ray) const;

    Vector apply_inv(const Vector& vec) const;
    Point apply_inv(const Point& pt) const;
    Normal apply_inv(const Normal& pt) const;
    DiffGeom apply_inv(const DiffGeom& dg) const;
    Ray apply_inv(const Ray& ray) const;
  };

  Transform translate(const Vector& delta);
  Transform scale(float x, float y, float z);
}

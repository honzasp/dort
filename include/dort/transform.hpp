#pragma once
#include "dort/geometry.hpp"
#include "dort/mat.hpp"
#include "dort/shape.hpp"

namespace dort {
  class Transform {
    Mat4x4 mat;
    Mat4x4 mat_inv;
  public:
    Transform(): mat(1.f), mat_inv(1.f) { }
    explicit Transform(const Mat4x4& mat, const Mat4x4& mat_inv):
      mat(mat), mat_inv(mat_inv) { }

    Transform inverse() const;
    Transform operator*(const Transform& trans) const;
    bool operator==(const Transform& trans) const {
      return this->mat == trans.mat && this->mat_inv == trans.mat_inv;
    }

    Vector apply(bool inv, const Vector& vec) const {
      return Vector(mul_mat_0(this->get_mat(inv), vec.v));
    }
    Point apply(bool inv, const Point& pt) const {
      return Point(mul_mat_1(this->get_mat(inv), pt.v));
    }
    Normal apply(bool inv, const Normal& norm) const {
      return Normal(mul_mat_transpose_0(this->get_mat(!inv), norm.v));
    }

    DiffGeom apply(bool inv, const DiffGeom& dg) const;
    Box apply(bool inv, const Box& box) const;
    Ray apply(bool inv, const Ray& ray) const;

    template<class T>
    T apply(const T& obj) const {
      return this->apply(false, obj);
    }

    template<class T>
    T apply_inv(const T& obj) const {
      return this->apply(true, obj);
    }

    const Mat4x4& get_mat(bool inv) const {
      return inv ? this->mat_inv : this->mat;
    }
  };

  Transform identity();
  Transform translate(const Vector& delta);
  Transform translate(float x, float y, float z);
  Transform scale(const Vector& s);
  Transform scale(float x, float y, float z);
  Transform scale(float x);
  Transform rotate_x(float angle);
  Transform rotate_y(float angle);
  Transform rotate_z(float angle);
  Transform perspective(float fov, float z_near, float z_far);
  Transform look_at(const Point& eye, const Point& look, const Vector& up);
  Transform stretch(const Point& p1, const Point& p2);
}

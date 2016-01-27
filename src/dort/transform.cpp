#include "dort/transform.hpp"

namespace dort {
  Transform Transform::inverse() const
  {
    return Transform(this->mat_inv, this->mat);
  }

  Transform Transform::operator*(const Transform& trans) const
  {
    return Transform(
        mul_mats(this->mat, trans.mat),
        mul_mats(trans.mat_inv, this->mat_inv));
  }

  Vector Transform::apply(const Vector& vec) const
  {
    return Vector(mul_mat_0(this->mat, vec.v));
  }

  Point Transform::apply(const Point& pt) const
  {
    return Point(mul_mat_1(this->mat, pt.v));
  }

  Normal Transform::apply(const Normal& norm) const
  {
    return Normal(mul_mat_transpose_0(this->mat_inv, norm.v));
  }
  
  DiffGeom Transform::apply(const DiffGeom& dg) const
  {
    DiffGeom ret = dg;
    ret.p = this->apply(dg.p);
    ret.nn = normalize(this->apply(dg.nn));
    return ret;
  }

  Box Transform::apply(const Box& box) const
  {
    Vector radius = (box.p_max - box.p_min) * 0.5f;
    Point mid = box.p_min + radius;

    Vector new_radius;
    for(uint32_t i = 0; i < 3; ++i) {
      Vector axis;
      axis.v[i] = 1.f;
      Vector new_axis(this->apply(axis));
      new_radius.v.x = max(new_radius.v.x, abs(new_axis.v.x));
      new_radius.v.y = max(new_radius.v.y, abs(new_axis.v.y));
      new_radius.v.z = max(new_radius.v.z, abs(new_axis.v.z));
    }

    return Box(mid - new_radius, mid + new_radius);
  }

  Ray Transform::apply(const Ray& ray) const
  {
    Ray ret(ray);
    ret.orig = this->apply(ray.orig);
    ret.dir = this->apply(ray.dir);
    return ret;
  }

  Vector Transform::apply_inv(const Vector& vec) const 
  {
    return Vector(mul_mat_0(this->mat_inv, vec.v));
  }

  Point Transform::apply_inv(const Point& pt) const
  {
    return Point(mul_mat_1(this->mat_inv, pt.v));
  }

  Normal Transform::apply_inv(const Normal& norm) const
  {
    return Normal(mul_mat_transpose_0(this->mat_inv, norm.v));
  }

  Transform translate(const Vector& delta)
  {
    Mat4x4 mat(1.f);
    mat.cols[3][0] = delta.v.x;
    mat.cols[3][1] = delta.v.y;
    mat.cols[3][2] = delta.v.z;

    Mat4x4 mat_inv(1.f);
    mat_inv.cols[3][0] = -delta.v.x;
    mat_inv.cols[3][1] = -delta.v.y;
    mat_inv.cols[3][2] = -delta.v.z;

    return Transform(mat, mat_inv);
  }

  Transform scale(float x, float y, float z)
  {
    Mat4x4 mat(1.f);
    mat.cols[0][0] = x;
    mat.cols[1][1] = y;
    mat.cols[2][2] = z;

    Mat4x4 mat_inv(1.f);
    mat_inv.cols[0][0] = 1.f / x;
    mat_inv.cols[1][1] = 1.f / y;
    mat_inv.cols[2][2] = 1.f / z;

    return Transform(mat, mat_inv);
  }

  DiffGeom Transform::apply_inv(const DiffGeom& dg) const
  {
    DiffGeom ret = dg;
    ret.p = this->apply_inv(dg.p);
    ret.nn = normalize(this->apply_inv(dg.nn));
    return ret;
  }

  Ray Transform::apply_inv(const Ray& ray) const
  {
    Ray ret(ray);
    ret.orig = this->apply_inv(ray.orig);
    ret.dir = this->apply_inv(ray.dir);
    return ret;
  }

}

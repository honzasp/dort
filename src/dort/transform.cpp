#include "dort/box.hpp"
#include "dort/transform.hpp"

namespace dort {
  Transform Transform::operator*(const Transform& trans) const {
    return Transform(
        mul_mats(this->mat, trans.mat),
        mul_mats(trans.mat_inv, this->mat_inv));
  }

  Transform Transform::inverse() const {
    return Transform(this->mat_inv, this->mat);
  }

  DiffGeom Transform::apply(bool inv, const DiffGeom& dg) const {
    DiffGeom ret(dg);
    ret.p = this->apply(inv, dg.p);
    ret.nn = normalize(this->apply(inv, dg.nn));
    ret.dpdu = this->apply(inv, dg.dpdu);
    ret.dpdv = this->apply(inv, dg.dpdv);
    return ret;
  }

  Box Transform::apply(bool inv, const Box& box) const {
    Vector radius = (box.p_max - box.p_min) * 0.5f;
    Point mid = box.p_min + radius;

    Vector new_radius;
    for(uint32_t i = 0; i < 3; ++i) {
      Vector axis;
      axis.v[i] = radius.v[i];
      new_radius = new_radius + abs(this->apply(inv, axis));
    }

    Point new_mid = this->apply(inv, mid);
    return Box(new_mid - new_radius, new_mid + new_radius);
  }

  Ray Transform::apply(bool inv, const Ray& ray) const {
    Ray ret(ray);
    ret.orig = this->apply(inv, ray.orig);
    ret.dir = this->apply(inv, ray.dir);
    return ret;
  }

  Transform identity() {
    return Transform(Mat4x4(1.f), Mat4x4(1.f));
  }

  Transform translate(const Vector& delta) {
    return translate(delta.v.x, delta.v.y, delta.v.z);
  }

  Transform translate(float x, float y, float z) {
    Mat4x4 mat(1.f);
    mat.cols[3][0] = x;
    mat.cols[3][1] = y;
    mat.cols[3][2] = z;

    Mat4x4 mat_inv(1.f);
    mat_inv.cols[3][0] = -x;
    mat_inv.cols[3][1] = -y;
    mat_inv.cols[3][2] = -z;

    return Transform(mat, mat_inv);
  }

  Transform scale(const Vector& s) {
    return scale(s.v.x, s.v.y, s.v.z);
  }

  Transform scale(float x, float y, float z) {
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

  Transform scale(float x) {
    return scale(x, x, x);
  }

  Transform rotate_x(float angle) {
    float sin_th = sin(angle);
    float cos_th = cos(angle);

    Mat4x4 mat(1.f);
    mat.cols[1][1] = cos_th;
    mat.cols[1][2] = sin_th;
    mat.cols[2][1] = -sin_th;
    mat.cols[2][2] = cos_th;
    return Transform(mat, transpose(mat));
  }

  Transform rotate_y(float angle) {
    float sin_th = sin(angle);
    float cos_th = cos(angle);

    Mat4x4 mat(1.f);
    mat.cols[0][0] = cos_th;
    mat.cols[0][2] = -sin_th;
    mat.cols[2][0] = sin_th;
    mat.cols[2][2] = cos_th;
    return Transform(mat, transpose(mat));
  }

  Transform rotate_z(float angle) {
    float sin_th = sin(angle);
    float cos_th = cos(angle);

    Mat4x4 mat(1.f);
    mat.cols[0][0] = cos_th;
    mat.cols[0][1] = sin_th;
    mat.cols[1][0] = -sin_th;
    mat.cols[1][1] = cos_th;
    return Transform(mat, transpose(mat));
  }

  Transform perspective(float fov, float z_near, float z_far) {
    Mat4x4 mat(1.f);
    mat.cols[2][2] = z_far / (z_far - z_near);
    mat.cols[2][3] = 1.f;
    mat.cols[3][2] = -(z_far * z_near) / (z_far - z_near);
    mat.cols[3][3] = 0.f;

    Mat4x4 inv_mat(1.f);
    inv_mat.cols[2][2] = 0.f;
    inv_mat.cols[2][3] = -(z_far - z_near) / (z_far * z_near);
    inv_mat.cols[3][2] = 1.f;
    inv_mat.cols[3][3] = 1.f / z_near;

    float inv_tan = 1.f / tan(fov / 2);
    return scale(inv_tan, inv_tan, 1.f) * Transform(mat, inv_mat);
  }

  Transform look_at(const Point& eye, const Point& look, const Vector& up) {
    Vector ortho_dir = normalize(look - eye);
    Vector ortho_right = normalize(cross(normalize(up), ortho_dir));
    Vector ortho_up = cross(ortho_dir, ortho_right);

    Mat4x4 mat(1.f);
    Mat4x4 mat_inv(1.f);
    for(uint32_t row = 0; row < 3; ++row) {
      mat_inv.cols[row][0] = mat.cols[0][row] = ortho_right.v[row];
      mat_inv.cols[row][1] = mat.cols[1][row] = ortho_up.v[row];
      mat_inv.cols[row][2] = mat.cols[2][row] = ortho_dir.v[row];
      mat.cols[3][row] = eye.v[row];
    }

    mat_inv.cols[3][0] = -dot(ortho_right.v, eye.v);
    mat_inv.cols[3][1] = -dot(ortho_up.v, eye.v);
    mat_inv.cols[3][2] = -dot(ortho_dir.v, eye.v);

    return Transform(mat, mat_inv);
  }
}

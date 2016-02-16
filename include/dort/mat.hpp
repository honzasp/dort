#pragma once
#include "dort/geometry.hpp"

namespace dort {
  struct Mat4x4 {
    float cols[4][4];

    Mat4x4(float diag = 0.f);
    Mat4x4(float cols[4][4]);
  };

  Mat4x4 transpose(const Mat4x4& mat);
  Mat4x4 mul_mats(const Mat4x4& m1, const Mat4x4& m2);
  Vec3 mul_mat_0(const Mat4x4& mat, const Vec3& v);
  Vec3 mul_mat_transpose_0(const Mat4x4& mat, const Vec3& v);
  Vec3 mul_mat_1(const Mat4x4& mat, const Vec3& v);
  bool operator==(const Mat4x4& mat1, const Mat4x4& mat2);
}

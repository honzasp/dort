#include "dort/mat.hpp"

namespace dort {
  Mat4x4::Mat4x4(float diag) {
    for(uint32_t col = 0; col < 4; ++col) {
      for(uint32_t row = 0; row < 4; ++row) {
        this->cols[col][row] = (row == col) ? diag : 0.f;
      }
    }
  }

  Mat4x4::Mat4x4(float cols[4][4]) {
    for(uint32_t col = 0; col < 4; ++col) {
      for(uint32_t row = 0; row < 4; ++row) {
        this->cols[col][row] = cols[col][row];
      }
    }
  }

  Mat4x4 transpose(const Mat4x4& mat) {
    Mat4x4 ret;
    for(uint32_t col = 0; col < 4; ++col) {
      for(uint32_t row = 0; row < 4; ++row) {
        ret.cols[col][row] = mat.cols[row][col];
      }
    }
    return ret;
  }

  Mat4x4 mul_mats(const Mat4x4& m1, const Mat4x4& m2) {
    Mat4x4 ret;
    for(uint32_t col = 0; col < 4; ++col) {
      for(uint32_t row = 0; row < 4; ++row) {
        for(uint32_t i = 0; i < 4; ++i) {
          ret.cols[col][row] += m1.cols[i][row] * m2.cols[col][i];
        }
      }
    }
    return ret;
  }

  Vec3 mul_mat_0(const Mat4x4& mat, const Vec3& v) {
    auto& m = mat.cols;
    return Vec3(
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2]);
  }

  Vec3 mul_mat_transpose_0(const Mat4x4& mat, const Vec3& v) {
    auto& m = mat.cols;
    return Vec3(
        m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2],
        m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
        m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]);
  }

  Vec3 mul_mat_1(const Mat4x4& mat, const Vec3& v) {
    auto& m = mat.cols;
    Vec3 ret = Vec3(
        m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0],
        m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1],
        m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2]);
    float w = 
        m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3];
    if(w == 1.f) {
      return ret;
    } else {
      return ret / w;
    }
  }
}

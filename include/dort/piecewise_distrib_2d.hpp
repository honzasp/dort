#pragma once
#include <vector>
#include "dort/dort.hpp"
#include "dort/slice.hpp"

namespace dort {
  class PiecewiseDistrib2d {
    uint32_t x_res;
    uint32_t y_res;
    std::vector<float> pdfs;
    std::vector<float> marginal_y_cdf;
    std::vector<float> cond_x_cdfs;
  public:
    PiecewiseDistrib2d() = default;
    PiecewiseDistrib2d(uint32_t x_res, uint32_t y_res, const std::vector<float>& values);
    Vec2 sample(Vec2 uv) const;
    float pdf(Vec2 xy) const;
    uint32_t area() const { return this->x_res * this->y_res; }
  private:
    static float sample_cdf_slice(slice<const float> cdf, float u, uint32_t& out_idx);
  };
}

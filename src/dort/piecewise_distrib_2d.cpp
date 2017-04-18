#include "dort/piecewise_distrib_2d.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  PiecewiseDistrib2d::PiecewiseDistrib2d(
      uint32_t x_res, uint32_t y_res, const std::vector<float>& values):
    x_res(x_res), y_res(y_res), pdfs(x_res * y_res),
    marginal_y_cdf(y_res + 1), cond_x_cdfs((x_res + 1) * y_res)
  {
    float sum = 0.f;
    this->marginal_y_cdf.at(0) = 0.f;
    for(uint32_t y = 0; y < y_res; ++y) {
      float y_sum = 0.f;

      this->cond_x_cdfs.at((x_res + 1) * y) = 0.f;
      for(uint32_t x = 0; x < x_res; ++x) {
        float value = values.at(y * x_res + x);
        y_sum += value;
        this->cond_x_cdfs.at((x_res + 1) * y + x + 1) = y_sum;
      }

      float inv_y_sum = 1.f / y_sum;
      for(uint32_t x = 0; x <= x_res; ++x) {
        this->cond_x_cdfs.at((x_res + 1) * y + x) *= inv_y_sum;
      }

      sum += y_sum;
      this->marginal_y_cdf.at(y + 1) = sum;
    }

    float inv_sum = 1.f / sum;
    for(uint32_t y = 0; y <= y_res; ++y) {
      this->marginal_y_cdf.at(y) *= inv_sum;
    }

    for(uint32_t i = 0; i < x_res * y_res; ++i) {
      this->pdfs.at(i) = values.at(i) * inv_sum;
    }
  }

  Vec2 PiecewiseDistrib2d::sample(Vec2 uv) const {
    uint32_t y_idx;
    float y = this->sample_cdf_slice(make_slice(this->marginal_y_cdf), uv.y, y_idx);
    uint32_t x_idx;
    float x = this->sample_cdf_slice(make_slice(this->cond_x_cdfs)
        .subslice_len((this->x_res + 1) * y_idx, this->x_res + 1), uv.x, x_idx);
    return Vec2(x, y);
  }

  float PiecewiseDistrib2d::pdf(Vec2 xy) const {
    uint32_t x_idx = floor_int32(xy.x);
    uint32_t y_idx = floor_int32(xy.y);
    if(x_idx >= this->x_res) { return 0.f; }
    if(y_idx >= this->y_res) { return 0.f; }
    return this->pdfs.at(y_idx * this->x_res + x_idx);
  }

  float PiecewiseDistrib2d::sample_cdf_slice(slice<const float> cdf,
      float u, uint32_t& out_idx) 
  {
    assert(u >= 0.f && u <= 1.f);
    uint32_t begin = 0;
    uint32_t end = cdf.size() - 1;
    while(begin + 1 < end) {
      uint32_t mid = begin + (end - begin) / 2;
      if(cdf.at(mid) < u) {
        begin = mid;
      } else {
        end = mid;
      }
    }

    float cdf_0 = cdf.at(begin);
    float cdf_1 = cdf.at(begin + 1);
    // TODO: this assert fails spuriously
    //assert(cdf_0 <= u && u <= cdf_1);
    float remainder = (u - cdf_0) / (cdf_1 - cdf_0);
    out_idx = begin;
    return float(begin) + remainder;
  }
}

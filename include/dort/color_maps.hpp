#pragma once
#include <vector>
#include "dort/texture.hpp"

namespace dort {
  std::shared_ptr<Texture<Spectrum, float>> grayscale_color_map();
  std::shared_ptr<Texture<Spectrum, float>> lerp_color_map(
      Spectrum color_0, Spectrum color_1);
  std::shared_ptr<Texture<Spectrum, float>> spline_color_map(
      const std::vector<Spectrum>& knots);

  template<class T>
  T eval_spline(const std::vector<T>& knots, float x);
}

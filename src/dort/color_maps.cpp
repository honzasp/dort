#include "dort/color_maps.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  std::shared_ptr<Texture<Spectrum, float>> grayscale_color_map() {
    return lerp_color_map(Spectrum(0.f), Spectrum(1.f));
  }

  std::shared_ptr<Texture<Spectrum, float>> lerp_color_map(
      Spectrum color_0, Spectrum color_1)
  {
    return make_texture<Spectrum, float>([=](float x) {
      return lerp(x, color_0, color_1);
    });
  }

  std::shared_ptr<Texture<Spectrum, float>> spline_color_map(
      const std::vector<Spectrum>& knots)
  {
    return make_texture<Spectrum, float>([=](float x) {
      return eval_spline(knots, x);
    });
  }

  template<class T>
  T eval_spline(const std::vector<T>& knots, float x) {
    float xx = clamp(x);
    int32_t segment_count = knots.size() - 3;
    assert(segment_count > 0);
    int32_t segment = floor_int32(xx * float(segment_count));

    T knot_0 = knots.at(segment);
    T knot_1 = knots.at(segment + 1);
    T knot_2 = knots.at(segment + 2);
    T knot_3 = knots.at(segment + 3);
    float seg_x = xx * float(segment_count) - float(segment);

    T c3 = -0.5f*knot_0 + 1.5f*knot_1 - 1.5f*knot_2 + 0.5f*knot_3;
    T c2 =       knot_0 - 2.5f*knot_1 + 2.0f*knot_2 - 0.5f*knot_3;
    T c1 = -0.5f*knot_0               + 0.5f*knot_2;
    T c0 =                     knot_1;
    return ((c3*seg_x + c2)*seg_x + c1)*seg_x + c0;
  }
}

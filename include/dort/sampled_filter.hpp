#pragma once
#include <vector>
#include "dort/dort.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  class SampledFilter {
    Vec2i sample_radius;
    Vec2 filter_to_sample_radius;
    std::vector<float> samples;
  public:
    Vec2 radius;

    SampledFilter(std::shared_ptr<Filter> filter, Vec2i sample_radius);
    float evaluate(Vec2 p) const;
  };
}

#pragma once
#include "dort/geometry.hpp"

namespace dort {
  Vector uniform_hemisphere_sample(float u1, float u2);
  float uniform_hemisphere_pdf(const Vector& w);
  Vector uniform_disk_sample(float u1, float u2);
  float uniform_disk_pdf();
  Vector cosine_hemisphere_sample(float u1, float u2);
  float cosine_hemisphere_pdf(const Vector& w);

  float mis_power_heuristic(int32_t num_a, float pdf_a,
      int32_t num_b, float pdf_b);

}

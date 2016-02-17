#pragma once
#include "dort/geometry.hpp"
#include "dort/slice.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  Vector uniform_hemisphere_sample(float u1, float u2);
  float uniform_hemisphere_pdf(const Vector& w);
  Vector uniform_sphere_sample(float u1, float u2);
  float uniform_sphere_pdf();
  Vector uniform_disk_sample(float u1, float u2);
  float uniform_disk_pdf();
  Vector cosine_hemisphere_sample(float u1, float u2);
  float cosine_hemisphere_pdf(const Vector& w);
  Vector uniform_cone_sample(float cos_theta_max, float u1, float u2);
  float uniform_cone_pdf(float cos_theta_max);

  float mis_power_heuristic(int32_t num_a, float pdf_a,
      int32_t num_b, float pdf_b);

  void stratified_sample(slice<float> out, Rng& rng);
  void stratified_sample(slice<Vec2> out,
      uint32_t x_strata, uint32_t y_strata, Rng& rng);
  void latin_hypercube_sample(slice<Vec2> out, Rng& rng);

  template<class T>
  void shuffle(slice<T> ary, Rng& rng);
}

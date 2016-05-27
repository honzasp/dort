#pragma once
#include "dort/geometry.hpp"
#include "dort/slice.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  Vec3 uniform_hemisphere_sample(float u1, float u2);
  float uniform_hemisphere_pdf(const Vec3& w);
  Vec3 uniform_sphere_sample(float u1, float u2);
  float uniform_sphere_pdf();
  Vec2 uniform_disk_sample(float u1, float u2);
  float uniform_disk_pdf();
  Vec3 cosine_hemisphere_sample(float u1, float u2);
  float cosine_hemisphere_pdf(float cos_theta);
  Vec3 uniform_cone_sample(float cos_theta_max, float u1, float u2);
  float uniform_cone_pdf(float cos_theta_max);
  Vec3 power_cosine_hemisphere_sample(float u1, float u2, float exponent);
  float power_cosine_hemisphere_pdf(float cos_theta, float exponent);

  float mis_power_heuristic(int32_t num_a, float pdf_a,
      int32_t num_b, float pdf_b);

  void stratified_sample(slice<float> out, Rng& rng);
  void stratified_sample(slice<Vec2> out,
      uint32_t x_strata, uint32_t y_strata, Rng& rng);
  void latin_hypercube_sample(slice<Vec2> out, Rng& rng);

  template<class T>
  void shuffle(slice<T> ary, Rng& rng);
  template<class T>
  void shuffle_chunks(slice<T> ary, uint32_t chunk_size, Rng& rng);
}

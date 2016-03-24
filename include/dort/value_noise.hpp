#pragma once
#include <vector>
#include "dort/texture.hpp"

namespace dort {
  struct ValueNoiseLayer {
    float frequency;
    float weight;
  };

  template<class Out, class In>
  Out value_noise(const std::vector<ValueNoiseLayer>& layers, In x);
  template<class Out> Out value_noise_layer(int32_t layer, float x);
  template<class Out> Out value_noise_layer(int32_t layer, Vec2 x);
  template<class Out> Out value_noise_layer(int32_t layer, Vec3 x);

  template<class Out> Out value_noise_lattice(int32_t layer, int32_t x);
  template<class Out> Out value_noise_lattice(int32_t layer, Vec2i x);
  template<class Out> Out value_noise_lattice(int32_t layer, Vec3i x);

  float value_noise_hash(int32_t x, int32_t y, int32_t z, int32_t w);
  template<class T>
  T value_noise_interp(float x, T t0, T t1);
}


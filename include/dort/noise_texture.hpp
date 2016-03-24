#pragma once
#include "dort/texture.hpp"
#include "dort/value_noise.hpp"

namespace dort {
  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
}


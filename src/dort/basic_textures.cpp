#include "dort/basic_textures.hpp"

namespace dort {
  std::shared_ptr<Texture<float, float>> gain_texture(float g) {
    return make_texture<float, float>([=](float x) { return gain(g, x); });
  }
  std::shared_ptr<Texture<float, float>> bias_texture(float b) {
    return make_texture<float, float>([=](float x) { return bias(b, x); });
  }
}

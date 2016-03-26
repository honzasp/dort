#pragma once
#include "dort/math.hpp"
#include "dort/spectrum.hpp"
#include "dort/texture.hpp"

namespace dort {
  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> lerp_texture(
      std::shared_ptr<Texture<float, In>> tex_t,
      std::shared_ptr<Texture<Out, In>> tex_0,
      std::shared_ptr<Texture<Out, In>> tex_1)
  {
    return make_texture<Out, In>([=](In x) {
      return lerp(tex_t->evaluate(x), tex_0->evaluate(x), tex_1->evaluate(x));
    });
  }

  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> checkerboard_texture(
      std::shared_ptr<Texture<Out, In>> even_tex,
      std::shared_ptr<Texture<Out, In>> odd_tex,
      float check_size)
  {
    float inv_check_size = 1.f / check_size;
    return make_texture<Out, In>([=](In x) {
      if(checkerboard_even_check(x, inv_check_size)) {
        return even_tex->evaluate(x);
      } else {
        return odd_tex->evaluate(x);
      }
    });
  }

  inline bool checkerboard_even_check(float x, float inv_check_size) {
    return int32_t(x * inv_check_size) % 2 == 0;
  }

  inline bool checkerboard_even_check(Vec2 v, float inv_check_size) {
    return (int32_t(v.x * inv_check_size) + int32_t(v.y * inv_check_size)) % 2 == 0;
  }

  inline bool checkerboard_even_check(Vec3 v, float inv_check_size) {
    return (int32_t(v.x * inv_check_size) + int32_t(v.y * inv_check_size) +
        int32_t(v.z * inv_check_size)) % 2 == 0;
  }

  std::shared_ptr<Texture<float, float>> gain_texture(float g);
  std::shared_ptr<Texture<float, float>> bias_texture(float b);
}

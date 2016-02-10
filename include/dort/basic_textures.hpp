#pragma once
#include "dort/math.hpp"
#include "dort/spectrum.hpp"
#include "dort/texture.hpp"

namespace dort {
  template<class T>
  std::shared_ptr<Texture<T>> const_texture(T value) {
    return make_texture<T>([=](const DiffGeom&) {
        return value;
      });
  }

  template<class T>
  std::shared_ptr<Texture<T>> add_texture(
      std::shared_ptr<Texture<T>> a, std::shared_ptr<Texture<T>> b)
  {
    return make_texture<T>([=](const DiffGeom& diff_geom) {
        return a->evaluate(diff_geom) + b->evaluate(diff_geom);
      });
  }

  template<class T>
  std::shared_ptr<Texture<T>> scale_texture(
      std::shared_ptr<Texture<float>> a, std::shared_ptr<Texture<T>> b)
  {
    return make_texture<T>([=](const DiffGeom& diff_geom) {
        return a->evaluate(diff_geom) * b->evaluate(diff_geom);
      });
  }

  template<class T>
  std::shared_ptr<Texture<T>> lerp_texture(
      std::shared_ptr<Texture<float>> t,
      std::shared_ptr<Texture<T>> tex_0,
      std::shared_ptr<Texture<T>> tex_1)
  {
    return make_texture<T>([=](const DiffGeom& diff_geom) {
        return lerp(t->evaluate(diff_geom),
          tex_0->evaluate(diff_geom), tex_1->evaluate(diff_geom));
      });
  }

  template<class T>
  std::shared_ptr<Texture<T>> checkerboard_texture(
      std::shared_ptr<TextureMap2d> texture_map,
      float check_size, T even_check, T odd_check)
  {
    float inv_check_size = 1.f / check_size;
    return make_texture<T>([=](const DiffGeom& diff_geom) {
        Tex2 st = texture_map->map(diff_geom);
        int32_t check_s = floor_int32(inv_check_size * st.s);
        int32_t check_t = floor_int32(inv_check_size * st.t);
        int32_t check = check_s + check_t;
        return (check % 2 == 0) ? even_check : odd_check;
      });
  }

  inline
  std::shared_ptr<Texture<Spectrum>> map_debug_texture(
      std::shared_ptr<TextureMap2d> texture_map)
  {
    return make_texture<Spectrum>([=](const DiffGeom& diff_geom) {
        Tex2 st = texture_map->map(diff_geom);
        return Spectrum(clamp(st.s), clamp(st.t), 0.f);
      });
  }
}

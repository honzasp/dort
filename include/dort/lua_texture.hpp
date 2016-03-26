#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char TEXTURE_TNAME[] = "dort.Texture";

  int lua_open_texture(lua_State* l);

  int lua_texture_compose(lua_State* l);
  template<class In> int lua_texture_make_const(lua_State* l);
  template<class OutIn> int lua_texture_make_identity(lua_State* l);
  template<class In> int lua_texture_make_checkerboard(lua_State* l);
  template<class Out, class In> int lua_texture_make_value_noise(lua_State* l);
  int lua_texture_make_lerp(lua_State* l);
  int lua_texture_make_image(lua_State* l);
  int lua_texture_add(lua_State* l);
  int lua_texture_mul(lua_State* l);

  int lua_texture_make_gain(lua_State* l);
  int lua_texture_make_bias(lua_State* l);

  int lua_texture_map_make_uv(lua_State* l);
  template<template<class> class TexMap>
  int lua_texture_map_make_2d(lua_State* l);
  int lua_texture_map_make_xyz(lua_State* l);

  int lua_texture_color_map_make_grayscale(lua_State* l);
  int lua_texture_color_map_make_lerp(lua_State* l);
  int lua_texture_color_map_make_spline(lua_State* l);

  int lua_texture_render_2d(lua_State* l);

  enum class LuaTextureOut {
    Float,
    Spectrum,
    Vec2,
    Vec3,
  };

  enum class LuaTextureIn {
    Geom,
    Float,
    Vec2,
    Vec3,
  };

  struct LuaTexture {
    LuaTextureOut out_type;
    LuaTextureIn in_type;
    std::shared_ptr<AnyTexture> texture;

    LuaTexture() = default;

    template<class Out, class In>
    LuaTexture(std::shared_ptr<Texture<Out, In>> tex);

    template<class Out, class In>
    std::shared_ptr<Texture<Out, In>> get() const;

    template<class Out, class In = const DiffGeom&>
    std::shared_ptr<Texture<Out, In>> check(lua_State* l) const;
  };

  const LuaTexture& lua_check_texture(lua_State* l, int idx);
  LuaTexture lua_cast_texture(lua_State* l, int idx);
  bool lua_test_texture(lua_State* l, int idx);
  void lua_push_texture(lua_State* l, const LuaTexture& tex);
}

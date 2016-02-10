#pragma once
#include "dort/lua.hpp"

namespace dort {
  constexpr const char TEXTURE_LIBNAME[] = "dort.texture";
  constexpr const char TEXTURE_FLOAT_TNAME[] = "dort.Texture.float";
  constexpr const char TEXTURE_SPECTRUM_TNAME[] = "dort.Texture.Spectrum";
  constexpr const char TEXTURE_MAP_2D_TNAME[] = "dort.TextureMap2d";

  int lua_open_texture(lua_State* l);

  int lua_texture_make_const(lua_State* l);
  int lua_texture_make_lerp(lua_State* l);
  int lua_texture_make_checkerboard(lua_State* l);
  int lua_texture_make_map_debug(lua_State* l);
  int lua_texture_make_image(lua_State* l);
  int lua_texture_eq(lua_State* l);
  int lua_texture_add(lua_State* l);
  int lua_texture_mul(lua_State* l);

  int lua_texture_map_2d_make_uv(lua_State* l);
  int lua_texture_map_2d_make_xy(lua_State* l);
  int lua_texture_map_2d_make_spherical(lua_State* l);
  int lua_texture_map_2d_make_cylindrical(lua_State* l);
  int lua_texture_map_2d_eq(lua_State* l);

  std::shared_ptr<Texture<float>> lua_cast_texture_float(lua_State* l, int idx);
  std::shared_ptr<Texture<float>> lua_check_texture_float(lua_State* l, int idx);
  bool lua_test_texture_float(lua_State* l, int idx);
  void lua_push_texture_float(lua_State* l, std::shared_ptr<Texture<float>> tex);

  std::shared_ptr<Texture<Spectrum>> lua_cast_texture_spectrum(lua_State* l, int idx);
  std::shared_ptr<Texture<Spectrum>> lua_check_texture_spectrum(lua_State* l, int idx);
  bool lua_test_texture_spectrum(lua_State* l, int idx);
  void lua_push_texture_spectrum(lua_State* l, std::shared_ptr<Texture<Spectrum>> tex);

  std::shared_ptr<TextureMap2d> lua_check_texture_map_2d(lua_State* l, int idx);
  bool lua_test_texture_map_2d(lua_State* l, int idx);
  void lua_push_texture_map_2d(lua_State* l, std::shared_ptr<TextureMap2d> map);

  enum class LuaTextureType {
    Unknown,
    Mismatch,
    Float,
    Spectrum,
  };

  LuaTextureType lua_join_texture_types(LuaTextureType t1, LuaTextureType t2);
  template<class... Args> LuaTextureType lua_join_texture_types(LuaTextureType t1,
      LuaTextureType t2, Args... ts)
  {
    return lua_join_texture_types(t1, lua_join_texture_types(t2, ts...));
  }

  LuaTextureType lua_infer_texture_type_from_param(lua_State* l,
      int params_idx, const char* param_name);
}

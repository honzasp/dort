#pragma once
#include "dort/geometry.hpp"
#include "dort/lua.hpp"

namespace dort {
  float lua_param_float(lua_State* l, int params_idx, const char* param_name);
  Point lua_param_point(lua_State* l, int params_idx, const char* param_name);
  Spectrum lua_param_spectrum(lua_State* l, int params_idx, const char* param_name);
  std::shared_ptr<Texture<float>> lua_param_texture_float(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<Texture<Spectrum>> lua_param_texture_spectrum(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<TextureMap2d> lua_param_texture_map_2d(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<Image<PixelRgb8>> lua_param_image(lua_State* l,
      int params_idx, const char* param_name);

  float lua_param_float_opt(lua_State* l, int params_idx,
      const char* param_name, float def);
  uint32_t lua_param_uint32_opt(lua_State* l, int params_idx,
      const char* param_name, uint32_t def);
  Spectrum lua_param_spectrum_opt(lua_State* l, int params_idx,
      const char* param_name, const Spectrum& def);
  std::shared_ptr<Texture<float>> lua_param_texture_float_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Texture<float>> def);
  std::shared_ptr<Texture<Spectrum>> lua_param_texture_spectrum_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Texture<Spectrum>> def);
  std::shared_ptr<TextureMap2d> lua_param_texture_map_2d_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<TextureMap2d> def);

  bool lua_param_is_set(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_float(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_spectrum(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_texture_float(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_texture_spectrum(lua_State* l, int params_idx, const char* param_name);

  void lua_params_check_unused(lua_State* l, int params_idx);
}

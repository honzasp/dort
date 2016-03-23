#pragma once
#include "dort/box_i.hpp"
#include "dort/geometry.hpp"
#include "dort/lua.hpp"
#include "dort/lua_texture.hpp"

namespace dort {
  float lua_param_float(lua_State* l, int params_idx, const char* param_name);
  uint32_t lua_param_uint32(lua_State* l, int params_idx, const char* param_name);
  Point lua_param_point(lua_State* l, int params_idx, const char* param_name);
  Spectrum lua_param_spectrum(lua_State* l, int params_idx, const char* param_name);
  Transform lua_param_transform(lua_State* l, int params_idx, const char* param_name);
  Boxi lua_param_boxi(lua_State* l, int params_idx, const char* param_name);
  LuaTexture lua_param_texture(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<Image<PixelRgb8>> lua_param_image(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<Shape> lua_param_shape(lua_State* l,
      int params_idx, const char* param_name);
  std::shared_ptr<Grid> lua_param_grid(lua_State* l,
      int params_idx, const char* param_name);

  float lua_param_float_opt(lua_State* l, int params_idx,
      const char* param_name, float def);
  uint32_t lua_param_uint32_opt(lua_State* l, int params_idx,
      const char* param_name, uint32_t def);
  Spectrum lua_param_spectrum_opt(lua_State* l, int params_idx,
      const char* param_name, const Spectrum& def);
  Transform lua_param_transform_opt(lua_State* l, int params_idx,
      const char* param_name, const Transform& def);
  LuaTexture lua_param_texture_opt(lua_State* l, int params_idx,
      const char* param_name, const LuaTexture& def);
  std::shared_ptr<Filter> lua_param_filter_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Filter> def);
  std::shared_ptr<Sampler> lua_param_sampler_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Sampler> def);
  std::string lua_param_string_opt(lua_State* l,
      int params_idx, const char* param_name, const std::string& def);

  bool lua_param_is_set(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_float(lua_State* l, int params_idx, const char* param_name);
  bool lua_param_is_spectrum(lua_State* l, int params_idx, const char* param_name);

  void lua_params_check_unused(lua_State* l, int params_idx);
}

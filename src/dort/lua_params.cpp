#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_texture.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  float lua_param_float(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    if(!lua_isnumber(l, -1)) {
      luaL_error(l, "Parameter '%s' must be a number", param_name);
    }
    float num = lua_tonumber(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return num;
  }
  Spectrum lua_param_spectrum(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Spectrum spec = lua_check_spectrum(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return spec;
  }
  std::shared_ptr<Texture<float>> lua_param_texture_float(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto tex = lua_cast_texture_float(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return tex;
  }
  std::shared_ptr<Texture<Spectrum>> lua_param_texture_spectrum(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto tex = lua_cast_texture_spectrum(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return tex;
  }
  std::shared_ptr<TextureMap2d> lua_param_texture_map_2d(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto map = lua_check_texture_map_2d(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return map;
  }
  std::shared_ptr<Image<PixelRgb8>> lua_param_image(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto image = lua_check_image(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return image;
  }

  float lua_param_float_opt(lua_State* l, int params_idx,
      const char* param_name, float def)
  {
    lua_getfield(l, params_idx, param_name);
    float num = lua_isnil(l, -1) ? def : lua_tonumber(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return num;
  }
  Spectrum lua_param_spectrum_opt(lua_State* l, int params_idx,
      const char* param_name, const Spectrum& def)
  {
    lua_getfield(l, params_idx, param_name);
    auto spec = lua_isnil(l, -1) ? def : lua_check_spectrum(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return spec;
  }
  std::shared_ptr<TextureMap2d> lua_param_texture_map_2d_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<TextureMap2d> def)
  {
    lua_getfield(l, params_idx, param_name);
    auto map = lua_isnil(l, -1) ? def : lua_check_texture_map_2d(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return map;
  }

  bool lua_param_is_set(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    bool is = !lua_isnil(l, -1); lua_pop(l, 1); return is;
  }
  bool lua_param_is_float(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    bool is = lua_isnumber(l, -1); lua_pop(l, 1); return is;
  }
  bool lua_param_is_spectrum(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    bool is = lua_test_spectrum(l, -1); lua_pop(l, 1); return is;
  }
  bool lua_param_is_texture_float(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    bool is = lua_test_texture_float(l, -1); lua_pop(l, 1); return is;
  }
  bool lua_param_is_texture_spectrum(lua_State* l,
      int params_idx, const char* param_name) 
  {
    lua_getfield(l, params_idx, param_name);
    bool is = lua_test_texture_spectrum(l, -1); lua_pop(l, 1); return is;
  }

  void lua_params_check_unused(lua_State* l, int params_idx) {
    lua_pushnil(l);
    if(lua_next(l, params_idx)) {
      const char* param_name = lua_tostring(l, -2);
      luaL_error(l, "Unused parameter '%s'", param_name);
    }
  }
}


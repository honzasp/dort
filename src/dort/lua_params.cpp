#include "dort/geometry.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_filter.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_grid.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_sampler.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_spectrum.hpp"
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
  uint32_t lua_param_uint32(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    if(!lua_isinteger(l, -1)) {
      luaL_error(l, "Parameter '%s' must be an integer", param_name);
    }
    int32_t num = lua_tointeger(l, -1);
    if(num < 0) {
      luaL_error(l, "Parameter '%s' must be unsigned", param_name);
    }
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return num;
  }
  Point lua_param_point(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Point pt = lua_check_point(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return pt;
  }
  Vector lua_param_vector(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Vector pt = lua_check_vector(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return pt;
  }
  Spectrum lua_param_spectrum(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Spectrum spec = lua_check_spectrum(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return spec;
  }
  Transform lua_param_transform(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Transform trans = lua_check_transform(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return trans;
  }
  Boxi lua_param_boxi(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    Boxi box = lua_check_boxi(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return box;
  }
  LuaTexture lua_param_texture(lua_State* l, int params_idx, const char* param_name) {
    lua_getfield(l, params_idx, param_name);
    LuaTexture box = lua_cast_texture(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return box;
  }
  std::shared_ptr<Image<PixelRgb8>> lua_param_image_8(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto image = lua_check_image_8(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return image;
  }
  std::shared_ptr<Shape> lua_param_shape(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto shape = lua_check_shape(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return shape;
  }
  std::shared_ptr<Grid> lua_param_grid(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto grid = lua_check_grid(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return grid;
  }
  std::shared_ptr<Material> lua_param_material(lua_State* l,
      int params_idx, const char* param_name)
  {
    lua_getfield(l, params_idx, param_name);
    auto material = lua_check_material(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return material;
  }

  float lua_param_float_opt(lua_State* l, int params_idx,
      const char* param_name, float def)
  {
    lua_getfield(l, params_idx, param_name);
    float num = lua_isnil(l, -1) ? def : lua_tonumber(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return num;
  }
  uint32_t lua_param_uint32_opt(lua_State* l, int params_idx,
      const char* param_name, uint32_t def)
  {
    lua_getfield(l, params_idx, param_name);
    int32_t num = lua_isnil(l, -1) ? def : lua_tointeger(l, -1);
    if(num < 0) {
      luaL_error(l, "Expected an unsigned number");
    }
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return uint32_t(num);
  }
  bool lua_param_bool_opt(lua_State* l, int params_idx,
      const char* param_name, bool def)
  {
    lua_getfield(l, params_idx, param_name);
    bool x = lua_isnil(l, -1) ? def : lua_toboolean(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return x;
  }
  std::string lua_param_string_opt(lua_State* l, int params_idx,
      const char* param_name, const std::string& def)
  {
    lua_getfield(l, params_idx, param_name);
    std::string x;
    if(lua_isnil(l, -1)) {
      x = def;
    } else {
      size_t len;
      const char* data = lua_tolstring(l, -1, &len);
      x = std::string(data, len);
    }
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return x;
  }
  Spectrum lua_param_spectrum_opt(lua_State* l, int params_idx,
      const char* param_name, const Spectrum& def)
  {
    lua_getfield(l, params_idx, param_name);
    auto spec = lua_isnil(l, -1) ? def : lua_check_spectrum(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return spec;
  }
  Transform lua_param_transform_opt(lua_State* l, int params_idx,
      const char* param_name, const Transform& def)
  {
    lua_getfield(l, params_idx, param_name);
    auto trans = lua_isnil(l, -1) ? def : lua_check_transform(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return trans;
  }
  LuaTexture lua_param_texture_opt(lua_State* l, int params_idx,
      const char* param_name, const LuaTexture& def)
  {
    lua_getfield(l, params_idx, param_name);
    auto tex = lua_isnil(l, -1) ? def : lua_cast_texture(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return tex ;
  }
  std::shared_ptr<Filter> lua_param_filter_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Filter> def)
  {
    lua_getfield(l, params_idx, param_name);
    auto filter = lua_isnil(l, -1) ? def : lua_check_filter(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return filter;
  }
  std::shared_ptr<Sampler> lua_param_sampler_opt(lua_State* l,
      int params_idx, const char* param_name, std::shared_ptr<Sampler> def)
  {
    lua_getfield(l, params_idx, param_name);
    auto sampler = lua_isnil(l, -1) ? def : lua_check_sampler(l, -1);
    lua_pushnil(l); lua_setfield(l, params_idx, param_name); lua_pop(l, 1);
    return sampler;
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

  void lua_params_check_unused(lua_State* l, int params_idx) {
    lua_pushnil(l);
    if(lua_next(l, params_idx)) {
      const char* param_name = lua_tostring(l, -2);
      luaL_error(l, "Unused parameter '%s'", param_name);
    }
  }
}


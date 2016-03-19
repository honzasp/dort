#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  constexpr const char FILTER_TNAME[] = "dort.Filter";

  int lua_open_filter(lua_State* l);

  int lua_filter_make_box(lua_State* l);
  int lua_filter_make_triangle(lua_State* l);
  int lua_filter_make_gaussian(lua_State* l);
  int lua_filter_make_mitchell(lua_State* l);
  int lua_filter_make_lanczos_sinc(lua_State* l);
  Vec2 lua_filter_param_radius(lua_State* l, int params_idx);

  std::shared_ptr<Filter> lua_check_filter(lua_State* l, int idx);
  bool lua_test_filter(lua_State* l, int idx);
  void lua_push_filter(lua_State* l, std::shared_ptr<Filter> filter);
}

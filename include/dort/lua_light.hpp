#pragma once
#include "dort/lua.hpp"

namespace dort {
  constexpr const char LIGHT_LIBNAME[] = "dort.light";
  constexpr const char LIGHT_TNAME[] = "dort.Light";

  int lua_open_light(lua_State* l);
  int lua_light_make_point(lua_State* l);
  int lua_light_make_diffuse(lua_State* l);

  std::shared_ptr<Light> lua_check_light(lua_State* l, int idx);
  bool lua_test_light(lua_State* l, int idx);
  void lua_push_light(lua_State* l, std::shared_ptr<Light> light);
}

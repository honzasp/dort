#pragma once
#include "dort/lua.hpp"
#include "dort/math.hpp"

namespace dort {
  static const char* const MATH_LIBNAME = "dort.math";
  int lua_open_math(lua_State* l);

  int lua_min(lua_State* l);
  int lua_max(lua_State* l);
}

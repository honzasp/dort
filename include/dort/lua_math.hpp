#pragma once
#include "dort/lua.hpp"
#include "dort/math.hpp"

namespace dort {
  constexpr const char RNG_TNAME[] = "dort.Rng";

  int lua_open_math(lua_State* l);

  int lua_math_min(lua_State* l);
  int lua_math_max(lua_State* l);
  int lua_math_make_rng(lua_State* l);
  int lua_math_rng_generate(lua_State* l);

  Rng& lua_check_rng(lua_State* l, int idx);
  bool lua_test_rng(lua_State* l, int idx);
  void lua_push_rng(lua_State* l, Rng rng);
}

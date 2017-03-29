#pragma once
#include <chrono>
#include "dort/lua.hpp"

namespace dort {
  constexpr const char TIME_POINT_TNAME[] = "dort.TimePoint";
  using LuaClock = std::chrono::high_resolution_clock;
  using LuaTimePoint = std::chrono::time_point<LuaClock>;

  int lua_open_chrono(lua_State* l);

  int lua_chrono_now(lua_State* l);
  int lua_chrono_difference_ns(lua_State* l);
  int lua_chrono_difference_s(lua_State* l);

  LuaTimePoint lua_check_time_point(lua_State* l, int idx);
  bool lua_test_time_point(lua_State* l, int idx);
  void lua_push_time_point(lua_State* l, LuaTimePoint tp);
}

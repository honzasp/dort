#pragma once
#include "dort/lua.hpp"

namespace dort {
  constexpr const char STATS_LIBNAME[] = "dort.stats";
  int lua_open_stats(lua_State* l);

  int lua_stats_reset(lua_State* l);
  int lua_stats_write_and_reset(lua_State* l);
}

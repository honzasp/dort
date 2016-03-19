#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char GRID_TNAME[] = "dort.Grid";

  int lua_open_grid(lua_State* l);
  int lua_grid_make(lua_State* l);
  int lua_grid_get(lua_State* l);
  int lua_grid_set(lua_State* l);

  std::shared_ptr<Grid> lua_check_grid(lua_State* l, int idx);
  bool lua_test_grid(lua_State* l, int idx);
  void lua_push_grid(lua_State* l, std::shared_ptr<Grid> grid);
}

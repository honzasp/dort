/// Voxel grid.
// `Grid` is an infinite grid of 16-bit signed integers.
// @module dort.grid
#include "dort/grid.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_grid.hpp"
#include "dort/lua_helpers.hpp"

namespace dort {
  int lua_open_grid(lua_State* l) {
    const luaL_Reg grid_methods[] = {
      {"get", lua_grid_get},
      {"set", lua_grid_set},
      {"__gc", lua_gc_shared_obj<Grid, GRID_TNAME>},
      {0, 0},
    };

    const luaL_Reg grid_funs[] = {
      {"make", lua_grid_make},
      {0, 0},
    };

    lua_register_type(l, GRID_TNAME, grid_methods);
    luaL_newlib(l, grid_funs);
    return 1;
  }

  /// Make an empty grid.
  // @function make
  int lua_grid_make(lua_State* l) {
    lua_push_grid(l, std::make_shared<Grid>());
    return 1;
  }

  /// @type Grid

  /// Get the value at `Vec3i` `pos`.
  // @function get
  // @param pos
  int lua_grid_get(lua_State* l) {
    auto grid = lua_check_grid(l, 1);
    Vec3i pos = lua_check_vec3i(l, 2);
    lua_pushinteger(l, grid->get(pos));
    return 1;
  }
  /// Set the value at `Vec3i` `pos` to `value`.
  // @function set
  // @param pos
  // @param value
  int lua_grid_set(lua_State* l) {
    auto grid = lua_check_grid(l, 1);
    Vec3i pos = lua_check_vec3i(l, 2);
    int16_t item = luaL_checkinteger(l, 3);
    grid->set(pos, item);
    return 0;
  }
  /// @section end

  std::shared_ptr<Grid> lua_check_grid(lua_State* l, int idx) {
    return lua_check_shared_obj<Grid, GRID_TNAME>(l, idx);
  }
  bool lua_test_grid(lua_State* l, int idx) {
    return lua_test_shared_obj<Grid, GRID_TNAME>(l, idx);
  }
  void lua_push_grid(lua_State* l, std::shared_ptr<Grid> grid) {
    lua_push_shared_obj<Grid, GRID_TNAME>(l, grid);
  }
}

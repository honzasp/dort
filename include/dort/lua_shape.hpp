#pragma once
#include "dort/lua.hpp"

namespace dort {
  int lua_open_shape(lua_State* l);

  int lua_shape_make(lua_State* l);
  int lua_shape_sphere(lua_State* l);
  int lua_shape_triangle(lua_State* l);
  int lua_shape_eq(lua_State* l);

  void lua_make_shape(lua_State* l, const char* shape_name, int params_idx);

  std::shared_ptr<Shape> lua_check_shape(lua_State* l, int idx);
  bool lua_test_shape(lua_State* l, int idx);
  void lua_push_shape(lua_State* l, std::shared_ptr<Shape> shape);
}

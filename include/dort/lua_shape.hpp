#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char SHAPE_LIBNAME[] = "dort.shape";
  constexpr const char SHAPE_TNAME[] = "dort.Shape";
  constexpr const char MESH_TNAME[] = "dort.Mesh";

  int lua_open_shape(lua_State* l);

  int lua_shape_make_sphere(lua_State* l);
  int lua_shape_make_disk(lua_State* l);
  int lua_shape_make_triangle(lua_State* l);
  int lua_shape_make_mesh(lua_State* l);

  std::shared_ptr<Shape> lua_check_shape(lua_State* l, int idx);
  bool lua_test_shape(lua_State* l, int idx);
  void lua_push_shape(lua_State* l, std::shared_ptr<Shape> shape);

  std::shared_ptr<Mesh> lua_check_mesh(lua_State* l, int idx);
  bool lua_test_mesh(lua_State* l, int idx);
  void lua_push_mesh(lua_State* l, std::shared_ptr<Mesh> mesh);
}

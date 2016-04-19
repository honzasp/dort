#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char SHAPE_TNAME[] = "dort.Shape";
  constexpr const char MESH_TNAME[] = "dort.Mesh";
  constexpr const char PLY_MESH_TNAME[] = "dort.PlyMesh";

  int lua_open_shape(lua_State* l);

  int lua_shape_make_sphere(lua_State* l);
  int lua_shape_make_disk(lua_State* l);
  int lua_shape_make_cube(lua_State* l);
  int lua_shape_make_mesh(lua_State* l);

  int lua_ply_mesh_read(lua_State* l);

  std::shared_ptr<Shape> lua_check_shape(lua_State* l, int idx);
  bool lua_test_shape(lua_State* l, int idx);
  void lua_push_shape(lua_State* l, std::shared_ptr<Shape> shape);

  std::shared_ptr<Mesh> lua_check_mesh(lua_State* l, int idx);
  bool lua_test_mesh(lua_State* l, int idx);
  void lua_push_mesh(lua_State* l, std::shared_ptr<Mesh> mesh);

  std::shared_ptr<PlyMesh> lua_check_ply_mesh(lua_State* l, int idx);
  bool lua_test_ply_mesh(lua_State* l, int idx);
  void lua_push_ply_mesh(lua_State* l, std::shared_ptr<PlyMesh> ply_mesh);
}

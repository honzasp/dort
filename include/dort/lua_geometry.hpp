#pragma once
#include "dort/geometry.hpp"
#include "dort/lua.hpp"

namespace dort {
  static const char* const GEOMETRY_LIBNAME = "dort.geometry";
  static const char* const VECTOR_TNAME = "dort.Vector";
  static const char* const POINT_TNAME = "dort.Point";

  int lua_open_geometry(lua_State* l);

  int lua_vector_make(lua_State* l);
  int lua_vector_get_x(lua_State* l);
  int lua_vector_get_y(lua_State* l);
  int lua_vector_get_z(lua_State* l);
  int lua_vector_tostring(lua_State* l);
  int lua_vector_add(lua_State* l);

  int lua_point_make(lua_State* l);
  int lua_point_get_x(lua_State* l);
  int lua_point_get_y(lua_State* l);
  int lua_point_get_z(lua_State* l);
  int lua_point_tostring(lua_State* l);

  Vector& lua_get_vector(lua_State* l, int idx);
  void lua_push_vector(lua_State* l, const Vector& vec);
  Point& lua_get_point(lua_State* l, int idx);
  void lua_push_point(lua_State* l, const Point& pt);
}

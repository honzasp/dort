#pragma once
#include "dort/geometry.hpp"
#include "dort/lua.hpp"
#include "dort/transform.hpp"

namespace dort {
  constexpr const char GEOMETRY_LIBNAME[] = "dort.geometry";
  constexpr const char VECTOR_TNAME[] = "dort.Vector";
  constexpr const char POINT_TNAME[] = "dort.Point";
  constexpr const char TRANSFORM_TNAME[] = "dort.Transform";

  int lua_open_geometry(lua_State* l);

  int lua_vector_make(lua_State* l);
  int lua_vector_get_x(lua_State* l);
  int lua_vector_get_y(lua_State* l);
  int lua_vector_get_z(lua_State* l);
  int lua_vector_tostring(lua_State* l);
  int lua_vector_add(lua_State* l);
  int lua_vector_eq(lua_State* l);

  int lua_point_make(lua_State* l);
  int lua_point_get_x(lua_State* l);
  int lua_point_get_y(lua_State* l);
  int lua_point_get_z(lua_State* l);
  int lua_point_tostring(lua_State* l);
  int lua_point_eq(lua_State* l);

  int lua_transform_identity(lua_State* l);
  int lua_transform_translate(lua_State* l);
  int lua_transform_scale(lua_State* l);
  int lua_transform_rotate_x(lua_State* l);
  int lua_transform_rotate_y(lua_State* l);
  int lua_transform_rotate_z(lua_State* l);

  int lua_transform_apply(lua_State* l);
  int lua_transform_apply_inv(lua_State* l);
  int lua_transform_inverse(lua_State* l);
  int lua_transform_mul(lua_State* l);
  int lua_transform_eq(lua_State* l);

  const Vector& lua_check_vector(lua_State* l, int idx);
  bool lua_test_vector(lua_State* l, int idx);
  void lua_push_vector(lua_State* l, const Vector& vec);

  const Point& lua_check_point(lua_State* l, int idx);
  bool lua_test_point(lua_State* l, int idx);
  void lua_push_point(lua_State* l, const Point& pt);

  const Transform& lua_check_transform(lua_State* l, int idx);
  bool lua_test_transform(lua_State* l, int idx);
  void lua_push_transform(lua_State* l, const Transform& trans);
}

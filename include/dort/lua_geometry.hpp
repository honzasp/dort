#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char VECTOR_TNAME[] = "dort.Vector";
  constexpr const char POINT_TNAME[] = "dort.Point";
  constexpr const char TRANSFORM_TNAME[] = "dort.Transform";
  constexpr const char VEC3I_TNAME[] = "dort.Vec3i";
  constexpr const char BOXI_TNAME[] = "dort.Boxi";

  int lua_open_geometry(lua_State* l);

  int lua_geometry_add(lua_State* l);
  int lua_geometry_sub(lua_State* l);
  int lua_geometry_eq(lua_State* l);

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

  int lua_transform_identity(lua_State* l);
  int lua_transform_translate(lua_State* l);
  int lua_transform_scale(lua_State* l);
  int lua_transform_rotate_x(lua_State* l);
  int lua_transform_rotate_y(lua_State* l);
  int lua_transform_rotate_z(lua_State* l);
  int lua_transform_look_at(lua_State* l);
  int lua_transform_stretch(lua_State* l);

  int lua_transform_tostring(lua_State* l);
  int lua_transform_apply(lua_State* l);
  int lua_transform_apply_inv(lua_State* l);
  int lua_transform_inverse(lua_State* l);
  int lua_transform_mul(lua_State* l);
  int lua_transform_eq(lua_State* l);

  int lua_vec3i_make(lua_State* l);
  int lua_vec3i_get_x(lua_State* l);
  int lua_vec3i_get_y(lua_State* l);
  int lua_vec3i_get_z(lua_State* l);
  int lua_vec3i_tostring(lua_State* l);

  int lua_boxi_make(lua_State* l);
  int lua_boxi_get_min(lua_State* l);
  int lua_boxi_get_max(lua_State* l);
  int lua_boxi_tostring(lua_State* l);
  int lua_boxi_contains(lua_State* l);

  const Vector& lua_check_vector(lua_State* l, int idx);
  bool lua_test_vector(lua_State* l, int idx);
  void lua_push_vector(lua_State* l, const Vector& vec);

  const Point& lua_check_point(lua_State* l, int idx);
  bool lua_test_point(lua_State* l, int idx);
  void lua_push_point(lua_State* l, const Point& pt);

  const Transform& lua_check_transform(lua_State* l, int idx);
  bool lua_test_transform(lua_State* l, int idx);
  void lua_push_transform(lua_State* l, const Transform& trans);

  const Vec3i& lua_check_vec3i(lua_State* l, int idx);
  bool lua_test_vec3i(lua_State* l, int idx);
  void lua_push_vec3i(lua_State* l, const Vec3i& vec3i);

  const Boxi& lua_check_boxi(lua_State* l, int idx);
  bool lua_test_boxi(lua_State* l, int idx);
  void lua_push_boxi(lua_State* l, const Boxi& box);
}

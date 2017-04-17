#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char VECTOR_TNAME[] = "dort.Vector";
  constexpr const char POINT_TNAME[] = "dort.Point";
  constexpr const char NORMAL_TNAME[] = "dort.Normal";
  constexpr const char TRANSFORM_TNAME[] = "dort.Transform";
  constexpr const char VEC3I_TNAME[] = "dort.Vec3i";
  constexpr const char VEC2I_TNAME[] = "dort.Vec2i";
  constexpr const char VEC2_TNAME[] = "dort.Vec2";
  constexpr const char BOXI_TNAME[] = "dort.Boxi";
  constexpr const char RECTI_TNAME[] = "dort.Recti";

  int lua_open_geometry(lua_State* l);

  int lua_geometry_add(lua_State* l);
  int lua_geometry_sub(lua_State* l);
  int lua_geometry_mul(lua_State* l);
  int lua_geometry_eq(lua_State* l);

  int lua_vector_make(lua_State* l);
  int lua_vector_get_x(lua_State* l);
  int lua_vector_get_y(lua_State* l);
  int lua_vector_get_z(lua_State* l);
  int lua_vector_tostring(lua_State* l);
  int lua_vector_length(lua_State* l);
  int lua_vector_cross(lua_State* l);

  int lua_point_make(lua_State* l);
  int lua_point_get_x(lua_State* l);
  int lua_point_get_y(lua_State* l);
  int lua_point_get_z(lua_State* l);
  int lua_point_tostring(lua_State* l);

  int lua_normal_make(lua_State* l);
  int lua_normal_get_x(lua_State* l);
  int lua_normal_get_y(lua_State* l);
  int lua_normal_get_z(lua_State* l);
  int lua_normal_tostring(lua_State* l);

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

  int lua_vec2i_make(lua_State* l);
  int lua_vec2i_get_x(lua_State* l);
  int lua_vec2i_get_y(lua_State* l);
  int lua_vec2i_tostring(lua_State* l);

  int lua_vec2_make(lua_State* l);
  int lua_vec2_get_x(lua_State* l);
  int lua_vec2_get_y(lua_State* l);
  int lua_vec2_tostring(lua_State* l);

  int lua_boxi_make(lua_State* l);
  int lua_boxi_get_min(lua_State* l);
  int lua_boxi_get_max(lua_State* l);
  int lua_boxi_tostring(lua_State* l);
  int lua_boxi_contains(lua_State* l);

  int lua_recti_make(lua_State* l);
  int lua_recti_get_min(lua_State* l);
  int lua_recti_get_max(lua_State* l);
  int lua_recti_tostring(lua_State* l);

  const Vector& lua_check_vector(lua_State* l, int idx);
  bool lua_test_vector(lua_State* l, int idx);
  void lua_push_vector(lua_State* l, const Vector& vec);

  const Point& lua_check_point(lua_State* l, int idx);
  bool lua_test_point(lua_State* l, int idx);
  void lua_push_point(lua_State* l, const Point& pt);

  const Normal& lua_check_normal(lua_State* l, int idx);
  bool lua_test_normal(lua_State* l, int idx);
  void lua_push_normal(lua_State* l, const Normal& n);

  const Transform& lua_check_transform(lua_State* l, int idx);
  bool lua_test_transform(lua_State* l, int idx);
  void lua_push_transform(lua_State* l, const Transform& trans);

  const Vec3i& lua_check_vec3i(lua_State* l, int idx);
  bool lua_test_vec3i(lua_State* l, int idx);
  void lua_push_vec3i(lua_State* l, const Vec3i& vec3i);

  const Vec2i& lua_check_vec2i(lua_State* l, int idx);
  bool lua_test_vec2i(lua_State* l, int idx);
  void lua_push_vec2i(lua_State* l, const Vec2i& vec2i);

  const Vec2& lua_check_vec2(lua_State* l, int idx);
  bool lua_test_vec2(lua_State* l, int idx);
  void lua_push_vec2(lua_State* l, Vec2 vec2);

  const Boxi& lua_check_boxi(lua_State* l, int idx);
  bool lua_test_boxi(lua_State* l, int idx);
  void lua_push_boxi(lua_State* l, const Boxi& box);

  const Recti& lua_check_recti(lua_State* l, int idx);
  bool lua_test_recti(lua_State* l, int idx);
  void lua_push_recti(lua_State* l, const Recti& rect);
}

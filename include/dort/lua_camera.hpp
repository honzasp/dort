#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char CAMERA_LIBNAME[] = "dort.camera";
  constexpr const char CAMERA_TNAME[] = "dort.Camera";

  int lua_open_camera(lua_State* l);

  int lua_camera_make_ortho(lua_State* l);
  int lua_camera_make_perspective(lua_State* l);

  std::shared_ptr<Camera> lua_check_camera(lua_State* l, int idx);
  bool lua_test_camera(lua_State* l, int idx);
  void lua_push_camera(lua_State* l, std::shared_ptr<Camera> camera);
}

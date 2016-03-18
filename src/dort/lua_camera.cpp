#include "dort/camera.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"

namespace dort {
  int lua_open_camera(lua_State* l) {
    const luaL_Reg camera_methods[] = {
      {"__gc", lua_gc_shared_obj<Camera, CAMERA_TNAME>},
      {0, 0},
    };

    const luaL_Reg camera_funs[] = {
      {"make_ortho", lua_camera_make_ortho},
      {"make_perspective", lua_camera_make_perspective},
      {0, 0},
    };

    lua_register_type(l, CAMERA_TNAME, camera_methods);
    luaL_newlib(l, camera_funs);
    return 1;
  }

  int lua_camera_make_ortho(lua_State* l) {
    int p = 1;
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    float screen_x = lua_param_float(l, p, "screen_x");
    float screen_y = lua_param_float(l, p, "screen_y");
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<OrthographicCamera>(
          lua_current_frame_transform(l) * transform,
          Vec2(screen_x, screen_y)));
    return 1;
  }

  int lua_camera_make_perspective(lua_State* l) {
    int p = 1;
    auto transform = lua_param_transform(l, p, "transform");
    float screen_x = lua_param_float_opt(l, p, "screen_x", 2.f);
    float screen_y = lua_param_float_opt(l, p, "screen_y", 2.f);
    float fov = lua_param_float_opt(l, p, "fov", PI * 0.5f);
    float z_near = lua_param_float_opt(l, p, "z_near", 1e0f);
    float z_far = lua_param_float_opt(l, p, "z_far", 1e3f);
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<PerspectiveCamera>(
          lua_current_frame_transform(l) * transform,
          Vec2(screen_x, screen_y), clamp(fov, 1e-2f, PI), z_near, z_far));
    return 1;
  }

  std::shared_ptr<Camera> lua_check_camera(lua_State* l, int idx) {
    return lua_check_shared_obj<Camera, CAMERA_TNAME>(l, idx);
  }
  bool lua_test_camera(lua_State* l, int idx) {
    return lua_test_shared_obj<Camera, CAMERA_TNAME>(l, idx);
  }
  void lua_push_camera(lua_State* l, std::shared_ptr<Camera> camera) {
    lua_push_shared_obj<Camera, CAMERA_TNAME>(l, camera);
  }
}

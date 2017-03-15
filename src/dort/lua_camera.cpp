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
      {"make_pinhole", lua_camera_make_pinhole},
      {0, 0},
    };

    lua_register_type(l, CAMERA_TNAME, camera_methods);
    luaL_newlib(l, camera_funs);
    return 1;
  }

  int lua_camera_make_ortho(lua_State* l) {
    return luaL_error(l, "Orthographic camera is not implemented yet");
    /*
    int p = 1;
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    float dimension = lua_param_float(l, p, "dimension");
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<OrthographicCamera>(transform, dimension));
    return 1;
    */
  }

  int lua_camera_make_pinhole(lua_State* l) {
    int p = 1;
    auto transform = lua_param_transform(l, p, "transform");
    float fov = lua_param_float_opt(l, p, "fov", PI * 0.5f);
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<PinholeCamera>(transform,
          clamp(fov, 1e-2f, PI)));
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

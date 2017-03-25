#include "dort/camera.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"
#include "dort/ortho_camera.hpp"
#include "dort/pinhole_camera.hpp"
#include "dort/thin_lens_camera.hpp"

namespace dort {
  int lua_open_camera(lua_State* l) {
    const luaL_Reg camera_methods[] = {
      {"__gc", lua_gc_shared_obj<Camera, CAMERA_TNAME>},
      {0, 0},
    };

    const luaL_Reg camera_funs[] = {
      {"make_ortho", lua_camera_make_ortho},
      {"make_pinhole", lua_camera_make_pinhole},
      {"make_thin_lens", lua_camera_make_thin_lens},
      {0, 0},
    };

    lua_register_type(l, CAMERA_TNAME, camera_methods);
    luaL_newlib(l, camera_funs);
    return 1;
  }

  int lua_camera_make_ortho(lua_State* l) {
    int p = 1;
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    float dimension = lua_param_float(l, p, "dimension");
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<OrthoCamera>(transform, dimension));
    return 1;
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

  int lua_camera_make_thin_lens(lua_State* l) {
    int p = 1;
    auto transform = lua_param_transform(l, p, "transform");
    float lens_radius = lua_param_float(l, p, "lens_radius");
    float focal_distance = lua_param_float(l, p, "focal_distance");
    float fov = lua_param_float_opt(l, p, "fov", PI * 0.5f);
    lua_params_check_unused(l, p);

    lua_push_camera(l, std::make_shared<ThinLensCamera>(transform,
          clamp(fov, 1e-2f, PI), lens_radius, focal_distance));
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

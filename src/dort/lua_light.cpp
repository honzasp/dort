#include "dort/diffuse_light.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_params.hpp"
#include "dort/point_light.hpp"

namespace dort {
  int lua_open_light(lua_State* l) {
    const luaL_Reg light_methods[] = {
      {"__gc", lua_gc_shared_obj<Light, LIGHT_TNAME>},
      {0, 0},
    };

    lua_register_type(l, LIGHT_TNAME, light_methods);
    lua_register(l, "point_light", lua_light_make_point);
    lua_register(l, "diffuse_light", lua_light_make_diffuse);
    return 0;
  }

  int lua_light_make_point(lua_State* l) {
    int p = 1;
    Point local_point = lua_param_point(l, p, "point");
    Spectrum intensity = lua_param_spectrum(l, p, "intensity");
    lua_params_check_unused(l, p);

    Point frame_point = lua_current_frame_transform(l).apply(local_point);
    lua_push_light(l, std::make_shared<PointLight>(frame_point, intensity));
    return 1;
  }

  int lua_light_make_diffuse(lua_State* l) {
    int p = 1;
    auto shape_to_world = lua_current_frame_transform(l);
    auto shape = lua_param_shape(l, p, "shape");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto num_samples = lua_param_uint32_opt(l, p, "num_samples", 8);
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<DiffuseLight>(
          shape, shape_to_world, radiance, num_samples));
    return 1;
  }

  std::shared_ptr<Light> lua_check_light(lua_State* l, int idx) {
    return lua_check_shared_obj<Light, LIGHT_TNAME>(l, idx);
  }
  bool lua_test_light(lua_State* l, int idx) {
    return lua_test_shared_obj<Light, LIGHT_TNAME>(l, idx);
  }
  void lua_push_light(lua_State* l, std::shared_ptr<Light> light) {
    lua_push_shared_obj<Light, LIGHT_TNAME>(l, light);
  }
}

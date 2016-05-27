#include "dort/diffuse_light.hpp"
#include "dort/directional_light.hpp"
#include "dort/infinite_light.hpp"
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

    const luaL_Reg light_funs[] = {
      {"make_point", lua_light_make_point},
      {"make_directional", lua_light_make_directional},
      {"make_diffuse", lua_light_make_diffuse},
      {"make_infinite", lua_light_make_infinite},
      {0, 0},
    };

    lua_register_type(l, LIGHT_TNAME, light_methods);
    luaL_newlib(l, light_funs);
    return 1;
  }

  int lua_light_make_point(lua_State* l) {
    int p = 1;
    auto point = lua_param_point(l, p, "point");
    auto intensity = lua_param_spectrum(l, p, "intensity");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<PointLight>(transform.apply(point), intensity));
    return 1;
  }

  int lua_light_make_directional(lua_State* l) {
    int p = 1;
    auto direction = lua_param_vector(l, p, "direction");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<DirectionalLight>(
          transform.apply(direction), radiance));
    return 1;
  }

  int lua_light_make_diffuse(lua_State* l) {
    int p = 1;
    auto shape = lua_param_shape(l, p, "shape");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto num_samples = lua_param_uint32_opt(l, p, "num_samples", 1);
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<DiffuseLight>(
          shape, transform, radiance, num_samples));
    return 1;
  }

  int lua_light_make_infinite(lua_State* l) {
    int p = 1;
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto num_samples = lua_param_uint32_opt(l, p, "num_samples", 1);
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<InfiniteLight>(radiance, num_samples));
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

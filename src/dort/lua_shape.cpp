#include "dort/disk.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/sphere.hpp"

namespace dort {
  int lua_open_shape(lua_State* l) {
    const luaL_Reg shape_methods[] = {
      {"__gc", lua_gc_shared_obj<Shape, SHAPE_TNAME>},
      {0, 0},
    };

    lua_register_type(l, SHAPE_TNAME, shape_methods);
    lua_register(l, "sphere", lua_shape_make_sphere);
    lua_register(l, "disk", lua_shape_make_disk);
    return 0;
  }

  int lua_shape_make_sphere(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<Sphere>(radius));
    return 1;
  }

  int lua_shape_make_disk(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    float z = lua_param_float_opt(l, p, "z", 0.f);
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<Disk>(radius, z));
    return 1;
  }

  std::shared_ptr<Shape> lua_check_shape(lua_State* l, int idx) {
    return lua_check_shared_obj<Shape, SHAPE_TNAME>(l, idx);
  }
  bool lua_test_shape(lua_State* l, int idx) {
    return lua_test_shared_obj<Shape, SHAPE_TNAME>(l, idx);
  }
  void lua_push_shape(lua_State* l, std::shared_ptr<Shape> shape) {
    lua_push_shared_obj<Shape, SHAPE_TNAME>(l, shape);
  }
}

/// Image filters for rendering.
// @module dort.filter
#include "dort/filter.hpp"
#include "dort/lua_filter.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"

namespace dort {
  int lua_open_filter(lua_State* l) {
    const luaL_Reg filter_methods[] = {
      {"__gc", lua_gc_shared_obj<Filter, FILTER_TNAME>},
      {0, 0},
    };

    const luaL_Reg filter_funs[] = {
      {"make_box", lua_filter_make_box},
      {"make_triangle", lua_filter_make_triangle},
      {"make_gaussian", lua_filter_make_gaussian},
      {"make_mitchell", lua_filter_make_mitchell},
      {"make_lanczos_sinc", lua_filter_make_lanczos_sinc},
      {0, 0},
    };

    lua_register_type(l, FILTER_TNAME, filter_methods);
    luaL_newlib(l, filter_funs);
    return 1;
  }

  /// Make a box filter.
  //
  // - `radius`, `radius_x`, `radius_y` -- set the extent of the filter in
  // pixels, either symmetric or separately for each dimension. The default
  // radius is 0.5 px.
  //
  // @function make_box
  // @param params
  int lua_filter_make_box(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<BoxFilter>(radius));
    return 1;
  }

  /// Make a triangle filter.
  //
  // - `radius`, `radius_x`, `radius_y` -- see `make_box`.
  //
  // @function make_triangle
  // @param params
  int lua_filter_make_triangle(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<TriangleFilter>(radius));
    return 1;
  }

  /// Make a Gaussian filter.
  //
  // - `radius`, `radius_x`, `radius_y` -- see `make_box`.
  // - `alpha` -- the exponent of the Gaussian (default 1).
  //
  // @function make_gaussian
  // @param params
  int lua_filter_make_gaussian(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    float alpha = lua_param_float_opt(l, p, "alpha", 1.f);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<GaussianFilter>(radius, alpha));
    return 1;
  }

  /// Make a Mitchell (polynomial) filter.
  //
  // - `radius`, `radius_x`, `radius_y` -- see `make_box`.
  // - `b`, `c` -- the parameters of the Mitchell polynomial (default 0.4 and
  // 0.3).
  //
  // @function make_mitchell
  // @param params
  int lua_filter_make_mitchell(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    float b = lua_param_float_opt(l, p, "b", 0.4f);
    float c = lua_param_float_opt(l, p, "c", 0.3f);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<MitchellFilter>(radius, b, c));
    return 1;
  }

  /// Make a Lanczos sinc filter.
  //
  // - `radius`, `radius_x`, `radius_y` -- see `make_box`.
  // - `tau` -- the tau parameter (default `sqrt(radius_x^2 + radius_y^2)`).
  //
  // @function make_lanczos_sinc
  // @param params
  int lua_filter_make_lanczos_sinc(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    float tau = lua_param_float_opt(l, p, "tau", length(radius));
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<LanczosSincFilter>(radius, tau));
    return 1;
  }

  Vec2 lua_filter_param_radius(lua_State* l, int params_idx) {
    float r = lua_param_float_opt(l, params_idx, "radius", 0.5f);
    float r_x = lua_param_float_opt(l, params_idx, "radius_x", r);
    float r_y = lua_param_float_opt(l, params_idx, "radius_y", r);
    return Vec2(r_x, r_y);
  }


  std::shared_ptr<Filter> lua_check_filter(lua_State* l, int idx) {
    return lua_check_shared_obj<Filter, FILTER_TNAME>(l, idx);
  }
  bool lua_test_filter(lua_State* l, int idx) {
    return lua_test_shared_obj<Filter, FILTER_TNAME>(l, idx);
  }
  void lua_push_filter(lua_State* l, std::shared_ptr<Filter> filter) {
    return lua_push_shared_obj<Filter, FILTER_TNAME>(l, std::move(filter));
  }
}

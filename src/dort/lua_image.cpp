#include "dort/filter.hpp"
#include "dort/image.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"

namespace dort {
  int lua_open_image(lua_State* l) {
    const luaL_Reg spectrum_methods[] = {
      {"r", lua_spectrum_red},
      {"g", lua_spectrum_green},
      {"b", lua_spectrum_blue},
      {"red", lua_spectrum_red},
      {"green", lua_spectrum_green},
      {"blue", lua_spectrum_blue},
      {"__add", lua_spectrum_add},
      {"__sub", lua_spectrum_sub},
      {"__mul", lua_spectrum_mul},
      {"__div", lua_spectrum_div},
      {"__eq", lua_spectrum_eq},
      {0, 0},
    };

    const luaL_Reg image_methods[] = {
      {"__eq", lua_image_eq},
      {"__gc", lua_gc_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>},
      {0, 0},
    };

    const luaL_Reg filter_methods[] = {
      {"__gc", lua_gc_shared_obj<Filter, FILTER_TNAME>},
      {0, 0},
    };

    lua_register_type(l, SPECTRUM_TNAME, spectrum_methods);
    lua_register_type(l, IMAGE_RGB8_TNAME, image_methods);
    lua_register_type(l, FILTER_TNAME, filter_methods);

    lua_register(l, "rgb", lua_spectrum_rgb);
    lua_register(l, "read_image", lua_image_read);
    lua_register(l, "write_png_image", lua_image_write_png);
    lua_register(l, "box_filter", lua_filter_make_box);
    lua_register(l, "triangle_filter", lua_filter_make_triangle);
    lua_register(l, "gaussian_filter", lua_filter_make_gaussian);
    lua_register(l, "mitchell_filter", lua_filter_make_mitchell);
    lua_register(l, "lanczos_sinc_filter", lua_filter_make_lanczos_sinc);

    return 0;
  }

  int lua_spectrum_rgb(lua_State* l) {
    lua_push_spectrum(l, Spectrum::from_rgb(
          luaL_checknumber(l, 1),
          luaL_checknumber(l, 2),
          luaL_checknumber(l, 3)));
    return 1;
  }
  int lua_spectrum_red(lua_State* l) {
    lua_pushnumber(l, lua_check_spectrum(l, 1).red());
    return 1;
  }
  int lua_spectrum_green(lua_State* l) {
    lua_pushnumber(l, lua_check_spectrum(l, 1).green());
    return 1;
  }
  int lua_spectrum_blue(lua_State* l) {
    lua_pushnumber(l, lua_check_spectrum(l, 1).blue());
    return 1;
  }
  int lua_spectrum_add(lua_State* l) {
    lua_push_spectrum(l, lua_check_spectrum(l, 1) + lua_check_spectrum(l, 2));
    return 1;
  }
  int lua_spectrum_sub(lua_State* l) {
    lua_push_spectrum(l, lua_check_spectrum(l, 1) - lua_check_spectrum(l, 2));
    return 1;
  }
  int lua_spectrum_mul(lua_State* l) {
    if(lua_isnumber(l, 2) && lua_test_spectrum(l, 1)) {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) * luaL_checknumber(l, 2));
    } else if(lua_isnumber(l, 1) && lua_test_spectrum(l, 2)) {
      lua_push_spectrum(l, luaL_checknumber(l, 1) * lua_check_spectrum(l, 2));
    } else {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) * lua_check_spectrum(l, 2));
    }
    return 1;
  }
  int lua_spectrum_div(lua_State* l) {
    if(lua_isnumber(l, 2) && lua_test_spectrum(l, 1)) {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) / luaL_checknumber(l, 2));
    } else if(lua_isnumber(l, 1) && lua_test_spectrum(l, 2)) {
      lua_push_spectrum(l, luaL_checknumber(l, 1) / lua_check_spectrum(l, 2));
    } else {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) / lua_check_spectrum(l, 2));
    }
    return 1;
  }
  int lua_spectrum_eq(lua_State* l) {
    if(lua_test_spectrum(l, 1) ^ lua_test_spectrum(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_spectrum(l, 1) == lua_check_spectrum(l, 2));
    }
    return 1;
  }

  int lua_image_read(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open image file for reading: %s", file_name);
    }
    auto image = std::make_shared<Image<PixelRgb8>>(read_image(file));
    std::fclose(file);
    lua_push_image(l, image);
    return 1;
  }

  int lua_image_write_png(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    auto image = lua_check_image(l, 2);
    FILE* file = std::fopen(file_name, "w");
    if(!file) {
      lua_pushfstring(l, "Could not open file for writing: %s", file_name);
      lua_error(l);
      return 0;
    }
    write_image_png(file, *image);
    std::fclose(file);
    return 0;
  }

  int lua_image_eq(lua_State* l) {
    if(lua_test_image(l, 1) ^ lua_test_image(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_image(l, 1).get() == lua_check_image(l, 2).get());
    }
    return 1;
  }

  int lua_filter_make_box(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<BoxFilter>(radius));
    return 1;
  }

  int lua_filter_make_triangle(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<TriangleFilter>(radius));
    return 1;
  }

  int lua_filter_make_gaussian(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    float alpha = lua_param_float_opt(l, p, "alpha", 1.f);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<GaussianFilter>(radius, alpha));
    return 1;
  }

  int lua_filter_make_mitchell(lua_State* l) {
    int p = 1;
    Vec2 radius = lua_filter_param_radius(l, p);
    float b = lua_param_float_opt(l, p, "b", 0.4f);
    float c = lua_param_float_opt(l, p, "c", 0.3f);
    lua_params_check_unused(l, p);
    lua_push_filter(l, std::make_shared<MitchellFilter>(radius, b, c));
    return 1;
  }

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

  const Spectrum& lua_check_spectrum(lua_State* l, int idx) {
    return lua_check_managed_obj<Spectrum, SPECTRUM_TNAME>(l, idx);
  }
  bool lua_test_spectrum(lua_State* l, int idx) {
    return lua_test_managed_obj<Spectrum, SPECTRUM_TNAME>(l, idx);
  }
  void lua_push_spectrum(lua_State* l, const Spectrum& spec) {
    return lua_push_managed_obj<Spectrum, SPECTRUM_TNAME>(l, spec);
  }

  std::shared_ptr<Image<PixelRgb8>> lua_check_image(lua_State* l, int idx) {
    return lua_check_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  bool lua_test_image(lua_State* l, int idx) {
    return lua_test_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  void lua_push_image(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image) {
    return lua_push_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, std::move(image));
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

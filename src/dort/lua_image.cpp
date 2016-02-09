#include "dort/image.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"

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
      {"__mul", lua_spectrum_mul},
      {"__eq", lua_spectrum_eq},
      {0, 0},
    };

    const luaL_Reg image_methods[] = {
      {"__eq", lua_image_eq},
      {"__gc", lua_gc_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>},
      {0, 0},
    };

    lua_register_type(l, SPECTRUM_TNAME, spectrum_methods);
    lua_register_type(l, IMAGE_RGB8_TNAME, image_methods);

    lua_register(l, "rgb", lua_spectrum_rgb);
    lua_register(l, "read_image", lua_image_read);
    lua_register(l, "write_png_image", lua_image_write_png);

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
  int lua_spectrum_mul(lua_State* l) {
    if(lua_isnumber(l, 2)) {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) * luaL_checknumber(l, 2));
    } else if(lua_test_spectrum(l, 2)) {
      lua_push_spectrum(l, lua_check_spectrum(l, 1) * lua_check_spectrum(l, 2));
    }
    return 1;
  }
  int lua_spectrum_eq(lua_State* l) {
    lua_pushboolean(l, lua_check_spectrum(l, 1) == lua_check_spectrum(l, 2));
    return 1;
  }

  int lua_image_read(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      lua_pushfstring(l, "Could not open file for reading: %s", file_name);
      lua_error(l);
      return 0;
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
    lua_pushboolean(l, lua_check_image(l, 1).get() == lua_check_image(l, 2).get());
    return 1;
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
}

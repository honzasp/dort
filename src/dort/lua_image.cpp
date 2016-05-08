#include "dort/image.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"

namespace dort {
  int lua_open_image(lua_State* l) {
    const luaL_Reg image8_methods[] = {
      {"__gc", lua_gc_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>},
      {0, 0},
    };

    const luaL_Reg imagef_methods[] = {
      {"__gc", lua_gc_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>},
      {0, 0},
    };

    const luaL_Reg image_funs[] = {
      {"read", lua_image_read},
      {"write_png", lua_image_write_png},
      {"write_rgbe", lua_image_write_rgbe},
      {0, 0},
    };

    lua_register_type(l, IMAGE_RGB8_TNAME, image8_methods);
    lua_register_type(l, IMAGE_RGBF_TNAME, imagef_methods);
    luaL_newlib(l, image_funs);
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
    lua_push_image_8(l, image);
    return 1;
  }

  int lua_image_write_png(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    auto image = lua_check_image_8(l, 2);
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

  int lua_image_write_rgbe(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    auto image = lua_check_image_f(l, 2);
    FILE* file = std::fopen(file_name, "w");
    if(!file) {
      lua_pushfstring(l, "Could not open file for writing: %s", file_name);
      lua_error(l);
      return 0;
    }
    write_image_rgbe(file, *image);
    std::fclose(file);
    return 0;
  }

  std::shared_ptr<Image<PixelRgb8>> lua_check_image_8(lua_State* l, int idx) {
    return lua_check_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  bool lua_test_image_8(lua_State* l, int idx) {
    return lua_test_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  void lua_push_image_8(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image) {
    return lua_push_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, std::move(image));
  }

  std::shared_ptr<Image<PixelRgbFloat>> lua_check_image_f(lua_State* l, int idx) {
    return lua_check_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(l, idx);
  }
  bool lua_test_image_f(lua_State* l, int idx) {
    return lua_test_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(l, idx);
  }
  void lua_push_image_f(lua_State* l, std::shared_ptr<Image<PixelRgbFloat>> image) {
    return lua_push_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(
        l, std::move(image));
  }
}

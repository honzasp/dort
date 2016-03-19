#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  constexpr const char IMAGE_RGB8_TNAME[] = "dort.Image.Rgb8";

  int lua_open_image(lua_State* l);

  int lua_image_read(lua_State* l);
  int lua_image_write_png(lua_State* l);
  int lua_image_eq(lua_State* l);

  std::shared_ptr<Image<PixelRgb8>> lua_check_image(lua_State* l, int idx);
  bool lua_test_image(lua_State* l, int idx);
  void lua_push_image(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image);
}

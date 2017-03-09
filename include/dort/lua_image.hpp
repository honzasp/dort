#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  constexpr const char IMAGE_RGB8_TNAME[] = "dort.Image.Rgb8";
  constexpr const char IMAGE_RGBF_TNAME[] = "dort.Image.RgbFloat";

  int lua_open_image(lua_State* l);

  int lua_image_read(lua_State* l);
  int lua_image_write_png(lua_State* l);
  int lua_image_write_rgbe(lua_State* l);
  int lua_image_tonemap_srgb(lua_State* l);
  int lua_image_tonemap_gamma(lua_State* l);

  std::shared_ptr<Image<PixelRgb8>> lua_check_image_8(lua_State* l, int idx);
  bool lua_test_image_8(lua_State* l, int idx);
  void lua_push_image_8(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image);

  std::shared_ptr<Image<PixelRgbFloat>> lua_check_image_f(lua_State* l, int idx);
  bool lua_test_image_f(lua_State* l, int idx);
  void lua_push_image_f(lua_State* l, std::shared_ptr<Image<PixelRgbFloat>> image);
}

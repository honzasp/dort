#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char IMAGE_LIBNAME[] = "dort.image";
  constexpr const char SPECTRUM_TNAME[] = "dort.Spectrum";
  constexpr const char IMAGE_RGB8_TNAME[] = "dort.Image.Rgb8";

  int lua_open_image(lua_State* l);

  int lua_spectrum_rgb(lua_State* l);
  int lua_spectrum_red(lua_State* l);
  int lua_spectrum_green(lua_State* l);
  int lua_spectrum_blue(lua_State* l);
  int lua_spectrum_add(lua_State* l);
  int lua_spectrum_sub(lua_State* l);
  int lua_spectrum_mul(lua_State* l);
  int lua_spectrum_div(lua_State* l);
  int lua_spectrum_eq(lua_State* l);

  int lua_image_read(lua_State* l);
  int lua_image_write_png(lua_State* l);
  int lua_image_eq(lua_State* l);

  const Spectrum& lua_check_spectrum(lua_State* l, int idx);
  bool lua_test_spectrum(lua_State* l, int idx);
  void lua_push_spectrum(lua_State* l, const Spectrum& spec);

  std::shared_ptr<Image<PixelRgb8>> lua_check_image(lua_State* l, int idx);
  bool lua_test_image(lua_State* l, int idx);
  void lua_push_image(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image);
}

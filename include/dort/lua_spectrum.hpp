#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  constexpr const char SPECTRUM_TNAME[] = "dort.Spectrum";

  int lua_open_spectrum(lua_State* l);

  int lua_spectrum_rgb(lua_State* l);
  int lua_spectrum_rgbh(lua_State* l);
  int lua_spectrum_red(lua_State* l);
  int lua_spectrum_green(lua_State* l);
  int lua_spectrum_blue(lua_State* l);
  int lua_spectrum_add(lua_State* l);
  int lua_spectrum_sub(lua_State* l);
  int lua_spectrum_mul(lua_State* l);
  int lua_spectrum_div(lua_State* l);
  int lua_spectrum_eq(lua_State* l);

  const Spectrum& lua_check_spectrum(lua_State* l, int idx);
  bool lua_test_spectrum(lua_State* l, int idx);
  void lua_push_spectrum(lua_State* l, const Spectrum& spec);
}


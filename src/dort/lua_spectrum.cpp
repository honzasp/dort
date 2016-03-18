#include "dort/spectrum.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_spectrum.hpp"

namespace dort {
  int lua_open_spectrum(lua_State* l) {
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

    const luaL_Reg spectrum_funs[] = {
      {"rgb", lua_spectrum_rgb},
      {0, 0},
    };

    lua_register_type(l, SPECTRUM_TNAME, spectrum_methods);
    luaL_newlib(l, spectrum_funs);
    return 1;
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

  const Spectrum& lua_check_spectrum(lua_State* l, int idx) {
    return lua_check_managed_obj<Spectrum, SPECTRUM_TNAME>(l, idx);
  }
  bool lua_test_spectrum(lua_State* l, int idx) {
    return lua_test_managed_obj<Spectrum, SPECTRUM_TNAME>(l, idx);
  }
  void lua_push_spectrum(lua_State* l, const Spectrum& spec) {
    return lua_push_managed_obj<Spectrum, SPECTRUM_TNAME>(l, spec);
  }
}


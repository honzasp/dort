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
      {"rgbh", lua_spectrum_rgbh},
      {0, 0},
    };

    lua_register_type(l, SPECTRUM_TNAME, spectrum_methods);
    luaL_newlib(l, spectrum_funs);
    return 1;
  }

  int lua_spectrum_rgb(lua_State* l) {
    if(lua_gettop(l) == 1) {
      float x = luaL_checknumber(l, 1);
      lua_push_spectrum(l, Spectrum(x));
    } else {
      lua_push_spectrum(l, Spectrum::from_rgb(
            luaL_checknumber(l, 1),
            luaL_checknumber(l, 2),
            luaL_checknumber(l, 3)));
    }
    return 1;
  }

  int lua_spectrum_rgbh(lua_State* l) {
    float red, green, blue;

    if(lua_isstring(l, 1)) {
      const char* str = luaL_checkstring(l, 1);
      if(str[0] == '#') {
        str = str + 1;
      }

      uint32_t hex = 0;
      for(uint32_t i = 0; i < 6; ++i) {
        uint32_t digit;
        if(str[i] >= 'a' && str[i] <= 'f') {
          digit = (str[i] - 'a') + 10;
        } else if(str[i] >= 'A' && str[i] <= 'F') {
          digit = (str[i] - 'A') + 10;
        } else if(str[i] >= '0' && str[i] <= '9') {
          digit = str[i] - '0';
        } else {
          return luaL_argerror(l, 1, "A hex-format color must have 6 hex-digits");
        }
        hex = (hex << 4) + digit;
      }

      red = float((hex >> 16) & 0xff) / 256.f;
      green = float((hex >> 8) & 0xff) / 256.f;
      blue = float(hex & 0xff) / 256.f;
    } else if(lua_isnumber(l, 1) && lua_isnumber(l, 2) && lua_isnumber(l, 3)) {
      red = luaL_checknumber(l, 1) / 256.f;
      green = luaL_checknumber(l, 2) / 256.f;
      blue = luaL_checknumber(l, 3) / 256.f;
    } else if(lua_isinteger(l, 1)) {
      int32_t hex = luaL_checkinteger(l, 1);
      red = float((hex >> 16) & 0xff) / 256.f;
      green = float((hex >> 8) & 0xff) / 256.f;
      blue = float(hex & 0xff) / 256.f;
    } else {
      return luaL_error(l, "Expected a string, an integer, or three numbers");
    }

    lua_push_spectrum(l, Spectrum::from_rgb(red, green, blue));
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


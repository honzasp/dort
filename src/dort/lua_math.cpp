#include "dort/lua_helpers.hpp"
#include "dort/lua_math.hpp"

namespace dort {
  int lua_open_math(lua_State* l) {
    const luaL_Reg math_funs[] = {
      {"sqrt", lua_wrap_unary_fun<sqrt>},
      {"exp", lua_wrap_unary_fun<exp>},
      {"floor", lua_wrap_unary_fun<floor>},
      {"ceil", lua_wrap_unary_fun<ceil>},
      {"abs", lua_wrap_unary_fun<abs>},
      {"sin", lua_wrap_unary_fun<sin>},
      {"cos", lua_wrap_unary_fun<cos>},
      {"tan", lua_wrap_unary_fun<tan>},
      {"asin", lua_wrap_unary_fun<asin>},
      {"acos", lua_wrap_unary_fun<acos>},
      {"square", lua_wrap_unary_fun<square>},
      {"atan2", lua_wrap_binary_fun<atan>},
      {"pow", lua_wrap_binary_fun<pow>},
      {"min", lua_min},
      {"max", lua_max},
      {0, 0},
    };

    auto constant = [&](const char* name, float value) {
      lua_pushnumber(l, value);
      lua_setfield(l, -2, name);
    };

    luaL_newlib(l, math_funs);
    constant("infinity", INFINITY);
    constant("pi", PI);
    constant("two_pi", TWO_PI);
    return 1;
  }

  int lua_min(lua_State* l) {
    int32_t arg_count = lua_gettop(l);
    float m = INFINITY;
    for(int32_t i = 1; i <= arg_count; ++i) {
      m = min(m, luaL_checknumber(l, i));
    }
    lua_pushnumber(l, m);
    return 1;
  }

  int lua_max(lua_State* l) {
    int32_t arg_count = lua_gettop(l);
    float m = -INFINITY;
    for(int32_t i = 1; i <= arg_count; ++i) {
      m = max(m, luaL_checknumber(l, i));
    }
    lua_pushnumber(l, m);
    return 1;
  }
}

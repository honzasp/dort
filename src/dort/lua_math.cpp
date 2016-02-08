#include "dort/lua_math.hpp"

namespace dort {
  template<float (*unary_fun)(float)>
  int lua_wrap_unary_fun(lua_State* l) {
    float a = luaL_checknumber(l, 1);
    lua_pushnumber(l, unary_fun(a));
    return 1;
  }

  template<float (*binary_fun)(float, float)>
  int lua_wrap_binary_fun(lua_State* l) {
    float a = luaL_checknumber(l, 1);
    float b = luaL_checknumber(l, 2);
    lua_pushnumber(l, binary_fun(a, b));
    return 1;
  }

  int lua_open_math(lua_State* l) {
    lua_register(l, "sqrt", lua_wrap_unary_fun<sqrt>);
    lua_register(l, "exp", lua_wrap_unary_fun<exp>);
    lua_register(l, "floor", lua_wrap_unary_fun<floor>);
    lua_register(l, "ceil", lua_wrap_unary_fun<ceil>);
    lua_register(l, "abs", lua_wrap_unary_fun<abs>);
    lua_register(l, "sin", lua_wrap_unary_fun<sin>);
    lua_register(l, "cos", lua_wrap_unary_fun<cos>);
    lua_register(l, "tan", lua_wrap_unary_fun<tan>);
    lua_register(l, "asin", lua_wrap_unary_fun<asin>);
    lua_register(l, "acos", lua_wrap_unary_fun<acos>);
    lua_register(l, "square", lua_wrap_unary_fun<square>);

    lua_register(l, "atan2", lua_wrap_binary_fun<atan>);
    lua_register(l, "pow", lua_wrap_binary_fun<pow>);

    lua_register(l, "min", lua_min);
    lua_register(l, "max", lua_max);

    auto lua_setconst = [](lua_State* l, const char* name, float value) {
      lua_pushnumber(l, value);
      lua_setglobal(l, name);
    };

    lua_setconst(l, "infinity", INFINITY);
    lua_setconst(l, "pi", PI);
    lua_setconst(l, "two_pi", TWO_PI);

    return 0;
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

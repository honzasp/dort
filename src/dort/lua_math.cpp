#include "dort/lua_helpers.hpp"
#include "dort/lua_math.hpp"

namespace dort {
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

    lua_register_const(l, "infinity", INFINITY);
    lua_register_const(l, "pi", PI);
    lua_register_const(l, "two_pi", TWO_PI);

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

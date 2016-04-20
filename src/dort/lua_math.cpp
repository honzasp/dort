#include "dort/lua_helpers.hpp"
#include "dort/lua_math.hpp"
#include "dort/rng.hpp"

namespace dort {
  int lua_open_math(lua_State* l) {
    const luaL_Reg rng_methods[] = {
      {"__call", lua_math_rng_generate},
      {"__gc", lua_gc_managed_obj<Rng, RNG_TNAME>},
      {0, 0},
    };

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
      {"min", lua_math_min},
      {"max", lua_math_max},
      {"make_rng", lua_math_make_rng},
      {0, 0},
    };

    auto constant = [&](const char* name, float value) {
      lua_pushnumber(l, value);
      lua_setfield(l, -2, name);
    };

    lua_register_type(l, RNG_TNAME, rng_methods);

    luaL_newlib(l, math_funs);
    constant("infinity", INFINITY);
    constant("pi", PI);
    constant("two_pi", TWO_PI);
    return 1;
  }

  int lua_math_min(lua_State* l) {
    int32_t arg_count = lua_gettop(l);
    float m = INFINITY;
    for(int32_t i = 1; i <= arg_count; ++i) {
      m = min(m, luaL_checknumber(l, i));
    }
    lua_pushnumber(l, m);
    return 1;
  }

  int lua_math_max(lua_State* l) {
    int32_t arg_count = lua_gettop(l);
    float m = -INFINITY;
    for(int32_t i = 1; i <= arg_count; ++i) {
      m = max(m, luaL_checknumber(l, i));
    }
    lua_pushnumber(l, m);
    return 1;
  }

  int lua_math_make_rng(lua_State* l) {
    uint32_t seed = luaL_checkinteger(l, 1);
    lua_push_rng(l, Rng(seed));
    return 1;
  }

  int lua_math_rng_generate(lua_State* l) {
    auto& rng = lua_check_rng(l, 1);

    float begin = 0.f;
    float end = 1.f;
    if(lua_gettop(l) >= 2) {
      begin = luaL_checknumber(l, 2);
    }
    if(lua_gettop(l) >= 3) {
      end = luaL_checknumber(l, 3);
    }

    float x = rng.uniform_float() * (end - begin) + begin;
    lua_pushnumber(l, x);
    return 1;
  }

  Rng& lua_check_rng(lua_State* l, int idx) {
    return lua_check_managed_obj<Rng, RNG_TNAME>(l, idx);
  }
  bool lua_test_rng(lua_State* l, int idx) {
    return lua_test_managed_obj<Rng, RNG_TNAME>(l, idx);
  }
  void lua_push_rng(lua_State* l, Rng rng) {
    return lua_push_managed_obj<Rng, RNG_TNAME>(l, std::move(rng));
  }
}

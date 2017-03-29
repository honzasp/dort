#include "dort/lua_chrono.hpp"
#include "dort/lua_helpers.hpp"

namespace dort {
  int lua_open_chrono(lua_State* l) {
    const luaL_Reg time_point_methods[] = {
      {0, 0},
    };

    const luaL_Reg chrono_funs[] = {
      {"now", lua_chrono_now},
      {"difference_ns", lua_chrono_difference_ns},
      {"difference_s", lua_chrono_difference_s},
      {0, 0},
    };

    lua_register_type(l, TIME_POINT_TNAME, time_point_methods);
    luaL_newlib(l, chrono_funs);
    return 1;
  }

  int lua_chrono_now(lua_State* l) {
    lua_push_time_point(l, LuaClock::now());
    return 1;
  }

  int lua_chrono_difference_ns(lua_State* l) {
    auto tp_1 = lua_check_time_point(l, 1);
    auto tp_2 = lua_check_time_point(l, 2);
    auto d = std::chrono::duration_cast<std::chrono::nanoseconds>(tp_2 - tp_1);
    lua_pushinteger(l, d.count());
    return 1;
  }

  int lua_chrono_difference_s(lua_State* l) {
    auto tp_1 = lua_check_time_point(l, 1);
    auto tp_2 = lua_check_time_point(l, 2);
    auto d = std::chrono::duration_cast<std::chrono::duration<float>>(tp_2 - tp_1);
    lua_pushnumber(l, d.count());
    return 1;
  }

  LuaTimePoint lua_check_time_point(lua_State* l, int idx) {
    return lua_check_managed_obj<LuaTimePoint, TIME_POINT_TNAME>(l, idx);
  }
  bool lua_test_time_point(lua_State* l, int idx) {
    return lua_test_managed_obj<LuaTimePoint, TIME_POINT_TNAME>(l, idx);
  }
  void lua_push_time_point(lua_State* l, LuaTimePoint tp) {
    return lua_push_managed_obj<LuaTimePoint, TIME_POINT_TNAME>(l, tp);
  }
}

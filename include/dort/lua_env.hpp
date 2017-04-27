#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  int lua_open_env(lua_State* l);

  int lua_env_get_argv(lua_State* l);
}

#include "dort/ctx.hpp"
#include "dort/lua_env.hpp"

namespace dort {
  int lua_open_env(lua_State* l) {
    const luaL_Reg env_funs[] = {
      {"get_argv", lua_env_get_argv},
      {0, 0},
    };

    luaL_newlib(l, env_funs);
    return 1;
  }

  int lua_env_get_argv(lua_State* l) {
    CtxG& ctx = *lua_get_ctx(l);

    lua_createtable(l, ctx.argv.size(), 0);
    for(uint32_t i = 0; i < ctx.argv.size(); ++i) {
      const std::string& arg = ctx.argv.at(i);
      lua_pushinteger(l, i + 1);
      lua_pushlstring(l, arg.data(), arg.size());
      lua_settable(l, -3);
    }

    return 1;
  }
}

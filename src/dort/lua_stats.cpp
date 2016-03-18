#include "dort/ctx.hpp"
#include "dort/lua_stats.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  int lua_open_stats(lua_State* l) {
    const luaL_Reg stats_funs[] = {
      {"reset", lua_stats_reset},
      {"write_and_reset", lua_stats_write_and_reset},
      {0, 0},
    };
    luaL_newlib(l, stats_funs);
    return 1;
  }

  int lua_stats_reset(lua_State* l) {
    CtxG* ctx = lua_get_ctx(l);
    ctx->pool->restart();
    stat_reset_global();
    return 0;
  }

  int lua_stats_write_and_reset(lua_State* l) {
    FILE* output = stderr;
    if(lua_gettop(l) >= 1) {
      const char* filename = luaL_checkstring(l, 1);
      output = std::fopen(filename, "r");
      if(!output) {
        return luaL_error(l, "Could not open file to write stats to: %s", filename);
      }
    }

    CtxG* ctx = lua_get_ctx(l);
    ctx->pool->restart();
    stat_report_global(output);
    stat_reset_global();

    if(output != stderr) {
      std::fclose(output);
    }
    return 0;
  }
}

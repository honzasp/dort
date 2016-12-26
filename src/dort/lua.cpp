#include "dort/dort.hpp"
#include "dort/ctx.hpp"
#include "dort/lua.hpp"

namespace dort {
  void lua_set_ctx(lua_State* l, CtxG* ctx) {
    lua_pushstring(l, CTX_REGISTRY_KEY);
    lua_pushlightuserdata(l, (void*)ctx);
    lua_settable(l, LUA_REGISTRYINDEX);
  }

  CtxG* lua_get_ctx(lua_State* l) {
    lua_pushstring(l, CTX_REGISTRY_KEY);
    lua_gettable(l, LUA_REGISTRYINDEX);
    void* ptr = lua_touserdata(l, -1);
    lua_pop(l, 1);
    assert(ptr != 0);
    return (CtxG*)ptr;
  }

  int lua_searcher(lua_State* l) {
    std::string library_name(luaL_checkstring(l, 1));
    return lua_searcher_search(l, "lua/" + library_name + ".lua");
  }

#ifdef DORT_USE_GTK
  extern "C" {
    int luaopen_lgi_corelgilua51(lua_State* L);
  }
#endif

  int lua_extern_searcher(lua_State* l) {
    std::string library_name(luaL_checkstring(l, 1));
#ifdef DORT_USE_GTK
    if(library_name == "lgi.corelgilua51") {
      lua_pushcfunction(l, luaopen_lgi_corelgilua51);
      lua_pushvalue(l, 1);
      return 2;
    }
#endif
    for(char& ch: library_name) {
      if(ch == '.') { ch = '/'; }
    }
    return lua_searcher_search(l, "lua/extern/" + library_name + ".lua");
  }

  int lua_searcher_search(lua_State* l, const std::string& path) {
    int status = luaL_loadfile(l, path.c_str());
    if(status == LUA_OK) {
      lua_pushvalue(l, 1);
      return 2;
    } else if(status == LUA_ERRFILE) {
      return 1;
    } else {
      return luaL_error(l, "Load error: %s", lua_tostring(l, -1));
    }
  }
}


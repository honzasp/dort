#include "dort/dort.hpp"
#include "dort/ctx.hpp"
#include "dort/lua.hpp"
#include "dort/lua_sources.hpp"

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

#ifdef DORT_USE_GTK
  extern "C" {
    int luaopen_lgi_corelgilua51(lua_State* L);
  }
#endif

  int lua_builtin_searcher(lua_State* l) {
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

    auto iter = lua_sources.find(library_name);
    if(iter == lua_sources.end()) {
      library_name = "extern/" + library_name;
      iter = lua_sources.find(library_name);
    }

    if(iter == lua_sources.end()) {
      lua_pushstring(l, "No builtin library with this name found");
      return 1;
    }

    const char* begin = iter->second.first;
    const char* end  = iter->second.second;
    int status = luaL_loadbufferx(l, begin, end - begin, library_name.c_str(), "bt");
    if(status != LUA_OK) {
      return 1;
    }
    return 1;
  }

  int lua_file_searcher(lua_State* l) {
    lua_pushstring(l, "Not implemented yet");
    return 1;
  }
}


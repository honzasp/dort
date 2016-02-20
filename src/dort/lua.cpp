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
}


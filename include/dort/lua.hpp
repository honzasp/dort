#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lua_zlib.h>
}

namespace dort {
  struct CtxG;

  constexpr const char CTX_REGISTRY_KEY[] = "dort.ctx";
  void lua_set_ctx(lua_State* l, CtxG* ctx);
  CtxG* lua_get_ctx(lua_State* l);

  int lua_searcher(lua_State* l);
  int lua_extern_searcher(lua_State* l);
  int lua_searcher_search(lua_State* l, const std::string& path);
}

#pragma once
#include <memory>
#include "dort/lua.hpp"

namespace dort {
  inline 
  void lua_register_type(lua_State* l, const char* tname, const luaL_Reg methods[]) {
    luaL_newmetatable(l, tname);
    lua_pushstring(l, "__index");
    lua_pushvalue(l, -2);
    lua_settable(l, -3);
    luaL_setfuncs(l, methods, 0);
    lua_pop(l, 1);
  }

  template<float (*unary_fun)(float)>
  int lua_wrap_unary_fun(lua_State* l) {
    float a = luaL_checknumber(l, 1);
    lua_pushnumber(l, unary_fun(a));
    return 1;
  }

  template<float (*binary_fun)(float, float)>
  int lua_wrap_binary_fun(lua_State* l) {
    float a = luaL_checknumber(l, 1);
    float b = luaL_checknumber(l, 2);
    lua_pushnumber(l, binary_fun(a, b));
    return 1;
  }

  template<class T, const char* tname>
  void lua_push_managed_obj(lua_State* l, const T& obj) {
    T* lua_obj = (T*)lua_newuserdata(l, sizeof(T));
    new (lua_obj) T(obj);
    luaL_getmetatable(l, tname);
    lua_setmetatable(l, -2);
  }

  template<class T, const char* tname>
  void lua_push_managed_gc_obj(lua_State* l, T&& obj) {
    T* lua_obj = (T*)lua_newuserdata(l, sizeof(T));
    new (lua_obj) T(std::move(obj));
    luaL_getmetatable(l, tname);
    lua_setmetatable(l, -2);
  }

  template<class T, const char* tname>
  int lua_gc_managed_obj(lua_State* l) {
    auto lua_ptr = (T*)luaL_checkudata(l, 1, tname);
    if(lua_ptr != 0) {
      lua_ptr->~T();
    }
    return 0;
  }

  template<class T, const char* tname>
  bool lua_test_managed_obj(lua_State* l, int idx) {
    return luaL_testudata(l, idx, tname) != 0;
  }

  template<class T, const char* tname>
  T& lua_check_managed_obj(lua_State* l, int idx) {
    return *((T*)luaL_checkudata(l, idx, tname));
  }

  template<class T, const char* tname>
  void lua_push_shared_obj(lua_State* l, std::shared_ptr<T> obj) {
    auto lua_ptr = (std::shared_ptr<T>*)lua_newuserdata(l, sizeof(std::shared_ptr<T>));
    new (lua_ptr) std::shared_ptr<T>(obj);
    luaL_getmetatable(l, tname);
    lua_setmetatable(l, -2);
  }

  template<class T, const char* tname>
  int lua_gc_shared_obj(lua_State* l) {
    auto lua_ptr = (std::shared_ptr<T>*)luaL_checkudata(l, 1, tname);
    if(lua_ptr != 0) {
      lua_ptr->~shared_ptr();
    }
    return 0;
  }

  template<class T, const char* tname>
  bool lua_test_shared_obj(lua_State* l, int idx) {
    return luaL_testudata(l, idx, tname) != 0;
  }

  template<class T, const char* tname>
  std::shared_ptr<T> lua_check_shared_obj(lua_State* l, int idx) {
    return *((std::shared_ptr<T>*)luaL_checkudata(l, idx, tname));
  }
}

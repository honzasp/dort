#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char SAMPLER_LIBNAME[] = "dort.sampler";
  constexpr const char SAMPLER_TNAME[] = "dort.Sampler";

  int lua_open_sampler(lua_State* l);

  int lua_sampler_make_random(lua_State* l);
  int lua_sampler_make_stratified(lua_State* l);

  std::shared_ptr<Sampler> lua_check_sampler(lua_State* l, int idx);
  bool lua_test_sampler(lua_State* l, int idx);
  void lua_push_sampler(lua_State* l, std::shared_ptr<Sampler> sampler);
}

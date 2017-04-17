#pragma once
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char MATERIAL_TNAME[] = "dort.Material";

  int lua_open_material(lua_State* l);

  int lua_material_make_lambert(lua_State* l);
  int lua_material_make_mirror(lua_State* l);
  int lua_material_make_phong(lua_State* l);
  int lua_material_make_bump(lua_State* l);

  std::shared_ptr<Material> lua_check_material(lua_State* l, int idx);
  bool lua_test_material(lua_State* l, int idx);
  void lua_push_material(lua_State* l, std::shared_ptr<Material> material);
}

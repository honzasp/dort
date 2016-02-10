#pragma once
#include "dort/lua.hpp"

namespace dort {
  struct BuilderFrame {
    std::vector<std::unique_ptr<Primitive>> prims;
  };

  struct BuilderState {
    Transform local_to_frame;
    std::shared_ptr<Material> material;
  };

  struct Builder {
    std::vector<BuilderFrame> frame_stack;
    BuilderFrame frame;
    std::vector<BuilderState> state_stack;
    BuilderState state;
    std::vector<std::unique_ptr<Light>> lights;
  };

  int lua_open_builder(lua_State* l);

  int lua_build_define_scene(lua_State* l);
  int lua_build_block(lua_State* l);
  int lua_build_instance(lua_State* l);
  int lua_build_set_transform(lua_State* l);
  int lua_build_set_material(lua_State* l);
  int lua_build_add_shape(lua_State* l);
  int lua_build_add_primitive(lua_State* l);
  int lua_build_add_light(lua_State* l);

  Builder& lua_get_current_builder(lua_State* l);
  void lua_set_current_builder(lua_State* l, Builder builder);
  void lua_unset_current_builder(lua_State* l);
}

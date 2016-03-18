#pragma once
#include <unordered_set>
#include <vector>
#include "dort/bvh_primitive.hpp"
#include "dort/lua.hpp"
#include "dort/transform.hpp"

namespace dort {
  constexpr const char BUILDER_LIBNAME[] = "dort.builder";
  constexpr const char SCENE_TNAME[] = "dort.Scene";
  constexpr const char PRIMITIVE_TNAME[] = "dort.Primitive";
  constexpr const char BUILDER_TNAME[] = "dort.Builder";
  constexpr const char BUILDER_REG_KEY[] = "dort.current_builder";

  struct BuilderFrame {
    std::vector<std::unique_ptr<Primitive>> prims;
  };

  struct BuilderState {
    Transform local_to_frame;
    std::shared_ptr<Material> material;
    BvhOpts bvh_opts;
  };

  struct Builder {
    std::vector<BuilderFrame> frame_stack;
    BuilderFrame frame;
    std::vector<BuilderState> state_stack;
    BuilderState state;
    std::vector<std::shared_ptr<Light>> lights;
    std::unordered_set<std::shared_ptr<Mesh>> meshes;
    std::unordered_set<std::shared_ptr<PrimitiveMesh>> prim_meshes;
    std::shared_ptr<Camera> camera;
  };

  int lua_open_builder(lua_State* l);

  int lua_builder_define_scene(lua_State* l);
  int lua_builder_define_block(lua_State* l);
  int lua_builder_define_instance(lua_State* l);
  int lua_builder_set_transform(lua_State* l);
  int lua_builder_set_material(lua_State* l);
  int lua_builder_set_camera(lua_State* l);
  int lua_builder_set_option(lua_State* l);
  int lua_builder_add_shape(lua_State* l);
  int lua_builder_add_primitive(lua_State* l);
  int lua_builder_add_light(lua_State* l);
  int lua_builder_add_read_ply_mesh(lua_State* l);
  int lua_builder_add_read_ply_mesh_as_bvh(lua_State* l);
  int lua_builder_add_ply_mesh(lua_State* l);
  int lua_builder_add_ply_mesh_as_bvh(lua_State* l);
  int lua_builder_add_voxel_grid(lua_State* l);

  int lua_scene_render(lua_State* l);
  int lua_scene_eq(lua_State* l);
  int lua_primitive_eq(lua_State* l);

  std::unique_ptr<Primitive> lua_make_aggregate(CtxG& ctx,
      const BuilderState& state, BuilderFrame frame);
  void lua_register_mesh(lua_State* l, std::shared_ptr<Mesh> mesh);
  void lua_register_prim_mesh(lua_State* l, std::shared_ptr<PrimitiveMesh> mesh);

  Builder& lua_get_current_builder(lua_State* l);
  void lua_set_current_builder(lua_State* l, Builder builder);
  void lua_unset_current_builder(lua_State* l);
  const Transform& lua_current_frame_transform(lua_State* l);

  std::shared_ptr<Scene> lua_check_scene(lua_State* l, int idx);
  bool lua_test_scene(lua_State* l, int idx);
  void lua_push_scene(lua_State* l, std::shared_ptr<Scene> scene);

  std::shared_ptr<Primitive> lua_check_primitive(lua_State* l, int idx);
  bool lua_test_primitive(lua_State* l, int idx);
  void lua_push_primitive(lua_State* l, std::shared_ptr<Primitive> prim);
}

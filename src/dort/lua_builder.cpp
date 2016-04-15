#include "dort/bvh_primitive.hpp"
#include "dort/camera.hpp"
#include "dort/direct_renderer.hpp"
#include "dort/film.hpp"
#include "dort/filter.hpp"
#include "dort/grid.hpp"
#include "dort/list_primitive.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_shape.hpp"
#include "dort/mesh_bvh_primitive.hpp"
#include "dort/mesh_triangle_primitive.hpp"
#include "dort/path_renderer.hpp"
#include "dort/ply_mesh.hpp"
#include "dort/random_sampler.hpp"
#include "dort/rng.hpp"
#include "dort/scene.hpp"
#include "dort/thread_pool.hpp"
#include "dort/voxel_grid_primitive.hpp"

namespace dort {
  int lua_open_builder(lua_State* l) {
    const luaL_Reg scene_methods[] = {
      {"__eq", lua_scene_eq},
      {"__gc", lua_gc_shared_obj<Scene, SCENE_TNAME>},
      {0, 0},
    };

    const luaL_Reg primitive_methods[] = {
      {"__eq", lua_primitive_eq},
      {"__gc", lua_gc_shared_obj<Primitive, PRIMITIVE_TNAME>},
      {0, 0},
    };

    const luaL_Reg builder_methods[] = {
      {"__gc", lua_gc_managed_obj<Builder, BUILDER_TNAME>},
      {0, 0},
    };

    const luaL_Reg builder_funs[] = {
      {"define_scene", lua_builder_define_scene},
      {"define_block", lua_builder_define_block},
      {"define_instance", lua_builder_define_instance},
      {"set_transform", lua_builder_set_transform},
      {"set_material", lua_builder_set_material},
      {"set_camera", lua_builder_set_camera},
      {"set_option", lua_builder_set_option},
      {"add_shape", lua_builder_add_shape},
      {"add_primitive", lua_builder_add_primitive},
      {"add_light", lua_builder_add_light},
      {"add_read_ply_mesh", lua_builder_add_read_ply_mesh},
      {"add_read_ply_mesh_as_bvh", lua_builder_add_read_ply_mesh_as_bvh},
      {"add_ply_mesh", lua_builder_add_ply_mesh},
      {"add_ply_mesh_as_bvh", lua_builder_add_ply_mesh_as_bvh},
      {"add_voxel_grid", lua_builder_add_voxel_grid},
      {"render", lua_scene_render},
      {0, 0},
    };

    lua_register_type(l, SCENE_TNAME, scene_methods);
    lua_register_type(l, PRIMITIVE_TNAME, primitive_methods);
    lua_register_type(l, BUILDER_TNAME, builder_methods);
    luaL_newlib(l, builder_funs);
    return 1;
  }

  int lua_builder_define_scene(lua_State* l) {
    lua_set_current_builder(l, Builder());
    lua_pushvalue(l, 1);
    lua_call(l, 0, 0);
    lua_settop(l, 0);
    lua_gc(l, LUA_GCCOLLECT, 0);

    Builder builder = std::move(lua_get_current_builder(l));
    lua_unset_current_builder(l);

    if(!builder.frame_stack.empty()) {
      luaL_error(l, "Frame stack is not empty");
    }
    if(!builder.state_stack.empty()) {
      luaL_error(l, "State stack is not empty");
    }
    if(!builder.camera) {
      luaL_error(l, "No camera is set in the scene");
    }

    auto scene = std::make_shared<Scene>();
    scene->primitive = lua_make_aggregate(*lua_get_ctx(l),
        builder.state, std::move(builder.frame));
    scene->lights = std::move(builder.lights);
    scene->camera = std::move(builder.camera);

    std::move(builder.meshes.begin(), builder.meshes.end(),
        std::back_inserter(scene->meshes));
    std::move(builder.prim_meshes.begin(), builder.prim_meshes.end(),
        std::back_inserter(scene->prim_meshes));

    lua_push_scene(l, std::move(scene));
    return 1;
  }

  int lua_builder_define_block(lua_State* l) {
    {
      Builder& builder = lua_get_current_builder(l);
      builder.state_stack.push_back(builder.state);
      lua_pushvalue(l, 1);
      lua_call(l, 0, 0);
    }

    {
      Builder& builder = lua_get_current_builder(l);
      if(builder.state_stack.empty()) {
        return luaL_error(l, "State stack is empty (not balanced)");
      }
      builder.state = builder.state_stack.back();
      builder.state_stack.pop_back();
      return 0;
    }
  }

  int lua_builder_define_instance(lua_State* l) {
    {
      Builder& builder = lua_get_current_builder(l);
      BuilderFrame new_frame;
      BuilderState new_state;
      new_state.material = builder.state.material;

      builder.state_stack.push_back(std::move(builder.state));
      builder.frame_stack.push_back(std::move(builder.frame));
      builder.state = std::move(new_state);
      builder.frame = std::move(new_frame);

      lua_pushvalue(l, 1);
      lua_call(l, 0, 0);
    }

    {
      Builder& builder = lua_get_current_builder(l);
      if(builder.frame_stack.empty()) {
        return luaL_error(l, "Frame stack is empty (not balanced)");
      } else if(builder.state_stack.empty()) {
        return luaL_error(l, "State stack is empty (not balanced)");
      }

      auto primitive = lua_make_aggregate(*lua_get_ctx(l),
          builder.state, std::move(builder.frame));
      builder.frame = std::move(builder.frame_stack.back());
      builder.state = std::move(builder.state_stack.back());
      builder.frame_stack.pop_back();
      builder.state_stack.pop_back();

      lua_push_primitive(l, std::shared_ptr<Primitive>(std::move(primitive)));
      return 1;
    }
  }

  int lua_builder_set_transform(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    const Transform& trans = lua_check_transform(l, 1);
    builder.state.local_to_frame = builder.state.local_to_frame * trans;
    return 0;
  }

  int lua_builder_set_material(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto material = lua_check_material(l, 1);
    builder.state.material = material;
    return 0;
  }

  int lua_builder_set_camera(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto camera = lua_check_camera(l, 1);
    builder.camera = camera;
    return 0;
  }

  int lua_builder_set_option(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    std::string option = luaL_checkstring(l, 1);
    if(option == "bvh split method") {
      std::string method = luaL_checkstring(l, 2);
      if(method == "sah") {
        builder.state.bvh_opts.split_method = BvhSplitMethod::Sah;
      } else if(method == "middle") {
        builder.state.bvh_opts.split_method = BvhSplitMethod::Middle;
      } else {
        luaL_error(l, "unknown bvh split method: %s", method.c_str());
      }
    } else if(option == "bvh leaf size") {
      builder.state.bvh_opts.leaf_size = luaL_checkinteger(l, 2);
    } else if(option == "bvh max leaf size") {
      builder.state.bvh_opts.max_leaf_size = luaL_checkinteger(l, 2);
    } else {
      luaL_error(l, "unknown option: %s", option.c_str());
    }
    return 0;
  }

  int lua_builder_add_shape(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto shape = lua_check_shape(l, 1);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;

    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    auto prim = std::make_unique<ShapePrimitive>(shape, material, nullptr, transform);
    builder.frame.prims.push_back(std::move(prim));
    return 0;
  }

  int lua_builder_add_primitive(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto primitive = lua_check_primitive(l, 1);
    auto transform = builder.state.local_to_frame;
    auto frame_prim = std::make_unique<FramePrimitive>(transform, primitive);
    builder.frame.prims.push_back(std::move(frame_prim));
    return 0;
  }

  int lua_builder_add_light(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    if(!builder.frame_stack.empty()) {
      luaL_error(l, "lights can only be added in the root frame");
      return 0;
    }
    builder.lights.push_back(lua_check_light(l, 1));
    return 0;
  }

  int lua_builder_add_read_ply_mesh(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    Builder& builder = lua_get_current_builder(l);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    auto prim_mesh = std::make_shared<PrimitiveMesh>();
    prim_mesh->material = material;
    bool ok = read_ply_to_mesh(file, transform, prim_mesh->mesh,
      [&](uint32_t index) {
        builder.frame.prims.push_back(
          std::make_unique<MeshTrianglePrimitive>(prim_mesh.get(), index));
      });
    std::fclose(file);

    if(!ok) {
      return luaL_error(l, "Could not read ply file: %s", file_name);
    }

    lua_register_prim_mesh(l, std::move(prim_mesh));
    return 0;
  }

  int lua_builder_add_read_ply_mesh_as_bvh(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    Builder& builder = lua_get_current_builder(l);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    std::vector<uint32_t> indices;
    auto mesh = std::make_shared<Mesh>();
    bool ok = read_ply_to_mesh(file, transform, *mesh,
      [&](uint32_t index) { indices.push_back(index); });
    std::fclose(file);

    if(!ok) {
      return luaL_error(l, "Could not read ply file: %s", file_name);
    }

    builder.frame.prims.push_back(std::make_unique<MeshBvhPrimitive>(
        mesh.get(), material, std::move(indices), builder.state.bvh_opts,
        *lua_get_ctx(l)->pool));
    lua_register_mesh(l, std::move(mesh));
    return 0;
  }

  int lua_builder_add_ply_mesh(lua_State* l) {
    auto ply_mesh = lua_check_ply_mesh(l, 1);

    Builder& builder = lua_get_current_builder(l);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    auto prim_mesh = std::make_shared<PrimitiveMesh>();
    prim_mesh->material = material;
    prim_mesh->mesh.vertices = ply_mesh->vertices;
    prim_mesh->mesh.points.reserve(ply_mesh->points.size());
    for(Point& pt: ply_mesh->points) {
      prim_mesh->mesh.points.push_back(transform.apply(pt));
    }

    for(uint32_t t = 0; t < ply_mesh->triangle_count; ++t) {
      builder.frame.prims.push_back(
          std::make_unique<MeshTrianglePrimitive>(prim_mesh.get(), t * 3));
    }
    lua_register_prim_mesh(l, std::move(prim_mesh));
    return 0;
  }

  int lua_builder_add_ply_mesh_as_bvh(lua_State* l) {
    auto ply_mesh = lua_check_ply_mesh(l, 1);

    Builder& builder = lua_get_current_builder(l);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    std::vector<uint32_t> indices;
    for(uint32_t t = 0; t < ply_mesh->triangle_count; ++t) {
      indices.push_back(t * 3);
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->vertices = ply_mesh->vertices;
    mesh->points.reserve(ply_mesh->points.size());
    for(Point& pt: ply_mesh->points) {
      mesh->points.push_back(transform.apply(pt));
    }

    builder.frame.prims.push_back(std::make_unique<MeshBvhPrimitive>(
        mesh.get(), material, std::move(indices), builder.state.bvh_opts,
        *lua_get_ctx(l)->pool));
    lua_register_mesh(l, std::move(mesh));
    return 0;
  }

  int lua_builder_add_voxel_grid(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto transform = builder.state.local_to_frame;

    int p = 1;
    std::shared_ptr<Grid> grid = lua_param_grid(l, p, "grid");
    Boxi box = lua_param_boxi(l, p, "box");

    std::vector<VoxelCubeMaterial> cube_voxels; {
      if(lua_getfield(l, p, "cube_voxels") != LUA_TTABLE) {
        luaL_error(l, "Expected an array of cube voxel materials");
      }
      int32_t array_len = lua_rawlen(l, -1);

      cube_voxels.push_back(VoxelCubeMaterial());
      for(int32_t i = 1; i <= array_len; ++i) {
        if(lua_rawgeti(l, -1, i) != LUA_TTABLE) {
          luaL_error(l, "Cube voxel material must be a table");
        }

        VoxelCubeMaterial material;
        for(int32_t j = 0; j < 6; ++j) {
          lua_rawgeti(l, -1, j + 1);
          material.faces.at(j) = lua_check_material(l, -1);
          lua_pop(l, 1);
        }

        cube_voxels.push_back(std::move(material));
        lua_pop(l, 1);
      }
      lua_pop(l, 1);

      lua_pushnil(l);
      lua_setfield(l, p, "cube_voxels");
    }

    std::vector<std::shared_ptr<Primitive>> prim_voxels; {
      if(lua_getfield(l, p, "primitive_voxels") != LUA_TTABLE) {
        luaL_error(l, "Expected an array of primitive voxel materials");
      }
      int32_t array_len = lua_rawlen(l, -1);

      prim_voxels.push_back(nullptr);
      for(int32_t i = 1; i <= array_len; ++i) {
        lua_rawgeti(l, -1, i);
        prim_voxels.push_back(lua_check_primitive(l, -1));
        lua_pop(l, 1);
      }
      lua_pop(l, 1);

      lua_pushnil(l);
      lua_setfield(l, p, "primitive_voxels");
    }

    lua_params_check_unused(l, p);

    VoxelMaterials voxel_materials;
    voxel_materials.cube_voxels = std::move(cube_voxels);
    voxel_materials.prim_voxels = std::move(prim_voxels);

    builder.frame.prims.push_back(std::make_unique<VoxelGridPrimitive>(
        box, *grid, transform, std::move(voxel_materials)));
    return 0;
  }

  int lua_scene_render(lua_State* l) {
    auto scene = lua_check_scene(l, 1);

    int p = 2;
    uint32_t x_res = lua_param_uint32_opt(l, p, "x_res", 800);
    uint32_t y_res = lua_param_uint32_opt(l, p, "y_res", 600);
    auto filter = lua_param_filter_opt(l, p, "filter", 
        std::make_shared<BoxFilter>(Vec2(0.5f, 0.5f)));
    auto sampler = lua_param_sampler_opt(l, p, "sampler",
        std::make_shared<RandomSampler>(1, 42));
    auto method = lua_param_string_opt(l, p, "renderer", "direct");

    auto film = std::make_shared<Film>(x_res, y_res, filter);

    std::shared_ptr<Renderer> renderer;
    if(method == "direct") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      renderer = std::make_shared<DirectRenderer>(
          scene, film, sampler, max_depth);
    } else if(method == "path") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      renderer = std::make_shared<PathRenderer>(
          scene, film, sampler, max_depth);
    } else {
      return luaL_error(l, "Unrecognized rendering method: %s", method.c_str());
    }

    lua_params_check_unused(l, p);

    CtxG& ctx = *lua_get_ctx(l);
    renderer->preprocess(ctx);
    renderer->render(ctx);

    auto image = std::make_shared<Image<PixelRgb8>>(film->to_image());
    lua_push_image(l, image);
    return 1;
  }

  int lua_scene_eq(lua_State* l) {
    if(lua_test_scene(l, 1) ^ lua_test_scene(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_scene(l, 1).get() == lua_check_scene(l, 2).get());
    }
    return 1;
  }

  int lua_primitive_eq(lua_State* l) {
    if(lua_test_primitive(l, 1) ^ lua_test_primitive(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_primitive(l, 1).get() ==
          lua_check_primitive(l, 2).get());
    }
    return 1;
  }


  std::unique_ptr<Primitive> lua_make_aggregate(CtxG& ctx,
      const BuilderState& state, BuilderFrame frame)
  {
    if(frame.prims.size() == 1) {
      return std::move(frame.prims.at(0));
    } else if(frame.prims.size() <= state.bvh_opts.leaf_size) {
      return std::make_unique<ListPrimitive>(std::move(frame.prims));
    } else {
      return std::make_unique<BvhPrimitive>(std::move(frame.prims),
          state.bvh_opts, *ctx.pool);
    }
  }

  void lua_register_mesh(lua_State* l, std::shared_ptr<Mesh> mesh) {
    Builder& builder = lua_get_current_builder(l);
    builder.meshes.insert(mesh);
  }

  void lua_register_prim_mesh(lua_State* l, std::shared_ptr<PrimitiveMesh> mesh) {
    Builder& builder = lua_get_current_builder(l);
    builder.prim_meshes.insert(mesh);
  }

  Builder& lua_get_current_builder(lua_State* l) {
    lua_getfield(l, LUA_REGISTRYINDEX, BUILDER_REG_KEY);
    if(lua_isnil(l, -1)) {
      luaL_error(l, "There is no current builder");
    }
    Builder& builder = lua_check_managed_obj<Builder, BUILDER_TNAME>(l, -1);
    lua_pop(l, 1);
    return builder;
  }

  void lua_set_current_builder(lua_State* l, Builder builder) {
    lua_push_managed_gc_obj<Builder, BUILDER_TNAME>(l, std::move(builder));
    lua_setfield(l, LUA_REGISTRYINDEX, BUILDER_REG_KEY);
  }

  void lua_unset_current_builder(lua_State* l) {
    lua_pushnil(l);
    lua_setfield(l, LUA_REGISTRYINDEX, BUILDER_REG_KEY);
  }

  const Transform& lua_current_frame_transform(lua_State* l) {
    return lua_get_current_builder(l).state.local_to_frame;
  }

  std::shared_ptr<Scene> lua_check_scene(lua_State* l, int idx) {
    return lua_check_shared_obj<Scene, SCENE_TNAME>(l, idx);
  }
  bool lua_test_scene(lua_State* l, int idx) {
    return lua_test_shared_obj<Scene, SCENE_TNAME>(l, idx);
  }
  void lua_push_scene(lua_State* l, std::shared_ptr<Scene> scene) {
    lua_push_shared_obj<Scene, SCENE_TNAME>(l, scene);
  }

  std::shared_ptr<Primitive> lua_check_primitive(lua_State* l, int idx) {
    return lua_check_shared_obj<Primitive, PRIMITIVE_TNAME>(l, idx);
  }
  bool lua_test_primitive(lua_State* l, int idx) {
    return lua_test_shared_obj<Primitive, PRIMITIVE_TNAME>(l, idx);
  }
  void lua_push_primitive(lua_State* l, std::shared_ptr<Primitive> prim) {
    lua_push_shared_obj<Primitive, PRIMITIVE_TNAME>(l, prim);
  }
}

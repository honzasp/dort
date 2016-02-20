#include "dort/bvh_primitive.hpp"
#include "dort/camera.hpp"
#include "dort/direct_renderer.hpp"
#include "dort/film.hpp"
#include "dort/filter.hpp"
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
#include "dort/ply_mesh.hpp"
#include "dort/random_sampler.hpp"
#include "dort/rng.hpp"
#include "dort/scene.hpp"
#include "dort/stratified_sampler.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  int lua_open_builder(lua_State* l) {
    const luaL_Reg scene_methods[] = {
      {"__eq", lua_scene_eq},
      {"__gc", lua_gc_shared_obj<Scene, SCENE_TNAME>},
      {0, 0},
    };

    const luaL_Reg sampler_methods[] = {
      {"__gc", lua_gc_shared_obj<Sampler, SAMPLER_TNAME>},
      {0, 0},
    };

    const luaL_Reg primitive_methods[] = {
      {"__eq", lua_primitive_eq},
      {"__gc", lua_gc_shared_obj<Primitive, PRIMITIVE_TNAME>},
      {0, 0},
    };

    const luaL_Reg ply_mesh_methods[] = {
      {"__gc", lua_gc_shared_obj<PlyMesh, PLY_MESH_TNAME>},
      {0, 0},
    };

    const luaL_Reg builder_methods[] = {
      {"__gc", lua_gc_managed_obj<Builder, BUILDER_TNAME>},
      {0, 0},
    };

    lua_register_type(l, SCENE_TNAME, scene_methods);
    lua_register_type(l, SAMPLER_TNAME, sampler_methods);
    lua_register_type(l, PRIMITIVE_TNAME, primitive_methods);
    lua_register_type(l, PLY_MESH_TNAME, ply_mesh_methods);
    lua_register_type(l, BUILDER_TNAME, builder_methods);

    lua_register(l, "define_scene", lua_build_define_scene);
    lua_register(l, "block", lua_build_block);
    lua_register(l, "instance", lua_build_instance);
    lua_register(l, "transform", lua_build_set_transform);
    lua_register(l, "material", lua_build_set_material);
    lua_register(l, "camera", lua_build_set_camera);
    lua_register(l, "option", lua_build_set_option);
    lua_register(l, "add_shape", lua_build_add_shape);
    lua_register(l, "add_primitive", lua_build_add_primitive);
    lua_register(l, "add_light", lua_build_add_light);
    lua_register(l, "add_ply_mesh", lua_build_add_ply_mesh);
    lua_register(l, "add_read_ply_mesh", lua_build_add_read_ply_mesh);

    lua_register(l, "render", lua_scene_render);
    lua_register(l, "random_sampler", lua_sampler_make_random);
    lua_register(l, "stratified_sampler", lua_sampler_make_stratified);
    lua_register(l, "read_ply_mesh", lua_ply_mesh_read);

    return 0;
  }

  int lua_build_define_scene(lua_State* l) {
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
    scene->triangle_meshes = std::move(builder.triangle_meshes);
    scene->camera = std::move(builder.camera);
    lua_push_scene(l, std::move(scene));
    return 1;
  }

  int lua_build_block(lua_State* l) {
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

  int lua_build_instance(lua_State* l) {
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

  int lua_build_set_transform(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    const Transform& trans = lua_check_transform(l, 1);
    builder.state.local_to_frame = builder.state.local_to_frame * trans;
    return 0;
  }

  int lua_build_set_material(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto material = lua_check_material(l, 1);
    builder.state.material = material;
    return 0;
  }

  int lua_build_set_camera(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto camera = lua_check_camera(l, 1);
    builder.camera = camera;
    return 0;
  }

  int lua_build_set_option(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    std::string option = luaL_checkstring(l, 1);
    if(option == "bvh split method") {
      std::string method = luaL_checkstring(l, 2);
      if(method == "sah") {
        builder.state.bvh_split_method = BvhSplitMethod::Sah;
      } else if(method == "median") {
        builder.state.bvh_split_method = BvhSplitMethod::Median;
      } else if(method == "middle") {
        builder.state.bvh_split_method = BvhSplitMethod::Middle;
      } else {
        luaL_error(l, "unknown bvh split method: %s", method.c_str());
      }
    } else if(option == "bvh max leaf size") {
      builder.state.bvh_max_leaf_size = luaL_checkinteger(l, 2);
    } else {
      luaL_error(l, "unknown option: %s", option.c_str());
    }
    return 0;
  }

  int lua_build_add_shape(lua_State* l) {
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

  int lua_build_add_primitive(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto primitive = lua_check_primitive(l, 1);
    auto transform = builder.state.local_to_frame;
    auto frame_prim = std::make_unique<FramePrimitive>(transform, primitive);
    builder.frame.prims.push_back(std::move(frame_prim));
    return 0;
  }

  int lua_build_add_light(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    if(!builder.frame_stack.empty()) {
      luaL_error(l, "lights can only be added in the root frame");
      return 0;
    }
    builder.lights.push_back(lua_check_light(l, 1));
    return 0;
  }

  int lua_build_add_ply_mesh(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto ply_mesh = lua_check_ply_mesh(l, 1);
    auto material = builder.state.material;
    auto transform = builder.state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    auto triangle_mesh = ply_to_triangle_mesh(*ply_mesh,
        material, nullptr, transform, builder.frame.prims);
    builder.triangle_meshes.push_back(std::move(triangle_mesh));
    return 0;
  }

  int lua_build_add_read_ply_mesh(lua_State* l) {
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

    auto triangle_mesh = read_ply_to_triangle_mesh(file,
        material, nullptr, transform, builder.frame.prims);
    std::fclose(file);

    if(!triangle_mesh) {
      return luaL_error(l, "Could not read ply file: %s", file_name);
    }

    builder.triangle_meshes.push_back(std::move(triangle_mesh));
    return 0;
  }


  int lua_scene_render(lua_State* l) {
    auto scene = lua_check_scene(l, 1);

    int p = 2;
    uint32_t x_res = lua_param_uint32_opt(l, p, "x_res", 800);
    uint32_t y_res = lua_param_uint32_opt(l, p, "y_res", 600);
    uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
    auto filter = lua_param_filter_opt(l, p, "filter", 
        std::make_shared<BoxFilter>(Vec2(0.5f, 0.5f)));
    auto sampler = lua_param_sampler_opt(l, p, "sampler",
        std::make_shared<RandomSampler>(1, 42));
    lua_params_check_unused(l, p);

    CtxG& ctx = *lua_get_ctx(l);
    auto film = std::make_shared<Film>(x_res, y_res, filter);
    DirectRenderer renderer(scene, film, sampler, max_depth);
    renderer.preprocess(ctx);
    renderer.render(ctx);

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


  int lua_sampler_make_random(lua_State* l) {
    int p = 1;
    uint32_t samples_per_pixel = lua_param_uint32_opt(l, p, "samples_per_pixel", 1);
    uint32_t seed = lua_param_uint32_opt(l, p, "seed", 1);
    lua_params_check_unused(l, p);
    
    lua_push_sampler(l, std::make_shared<RandomSampler>(
          samples_per_pixel, Rng(seed)));
    return 1;
  }

  int lua_sampler_make_stratified(lua_State* l) {
    int p = 1;

    uint32_t samples_per_x = lua_param_uint32_opt(l, p, "samples_per_x", 1);
    uint32_t samples_per_y = lua_param_uint32_opt(l, p, "samples_per_y", 1);
    uint32_t seed = lua_param_uint32_opt(l, p, "seed", 1);
    lua_params_check_unused(l, p);

    lua_push_sampler(l, std::make_shared<StratifiedSampler>(
          samples_per_x, samples_per_y, Rng(seed)));
    return 1;
  }

  int lua_ply_mesh_read(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    auto ply_mesh = std::make_shared<PlyMesh>();
    bool success = read_ply(file, *ply_mesh);
    std::fclose(file);

    if(!success) {
      return luaL_error(l, "Could not read ply file: %s", file_name);
    }
    lua_push_ply_mesh(l, ply_mesh);
    return 1;
  }

  std::unique_ptr<Primitive> lua_make_aggregate(CtxG& ctx,
      const BuilderState& state, BuilderFrame frame)
  {
    return std::make_unique<BvhPrimitive>(std::move(frame.prims),
        state.bvh_max_leaf_size, state.bvh_split_method,
        *ctx.pool);
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

  std::shared_ptr<Sampler> lua_check_sampler(lua_State* l, int idx) {
    return lua_check_shared_obj<Sampler, SAMPLER_TNAME>(l, idx);
  }
  bool lua_test_sampler(lua_State* l, int idx) {
    return lua_test_shared_obj<Sampler, SAMPLER_TNAME>(l, idx);
  }
  void lua_push_sampler(lua_State* l, std::shared_ptr<Sampler> sampler) {
    lua_push_shared_obj<Sampler, SAMPLER_TNAME>(l, sampler);
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

  std::shared_ptr<PlyMesh> lua_check_ply_mesh(lua_State* l, int idx) {
    return lua_check_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, idx);
  }
  bool lua_test_ply_mesh(lua_State* l, int idx) {
    return lua_test_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, idx);
  }
  void lua_push_ply_mesh(lua_State* l, std::shared_ptr<PlyMesh> ply) {
    lua_push_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, ply);
  }
}

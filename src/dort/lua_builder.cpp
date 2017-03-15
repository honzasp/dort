#include "dort/bvh_primitive.hpp"
#include "dort/camera.hpp"
#include "dort/ctx.hpp"
#include "dort/grid.hpp"
#include "dort/light.hpp"
#include "dort/list_primitive.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_shape.hpp"
#include "dort/mesh_bvh_primitive.hpp"
#include "dort/mesh_triangle_primitive.hpp"
#include "dort/ply_mesh.hpp"
#include "dort/scene.hpp"
#include "dort/thread_pool.hpp"
#include "dort/triangle_shape.hpp"
#include "dort/triangle_shape_primitive.hpp"
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
      {"__gc", lua_gc_shared_obj<Builder, BUILDER_TNAME>},
      {0, 0},
    };

    const luaL_Reg builder_funs[] = {
      {"make", lua_builder_make},
      {"build_scene", lua_builder_build_scene},
      {"push_state", lua_builder_push_state},
      {"pop_state", lua_builder_pop_state},
      {"push_frame", lua_builder_push_frame},
      {"pop_frame", lua_builder_pop_frame},
      {"set_transform", lua_builder_set_transform},
      {"get_transform", lua_builder_get_transform},
      {"set_material", lua_builder_set_material},
      {"get_material", lua_builder_get_material},
      {"set_camera", lua_builder_set_camera},
      {"set_option", lua_builder_set_option},
      {"add_shape", lua_builder_add_shape},
      {"add_primitive", lua_builder_add_primitive},
      {"add_triangle", lua_builder_add_triangle},
      {"add_light", lua_builder_add_light},
      {"add_read_ply_mesh", lua_builder_add_read_ply_mesh},
      {"add_read_ply_mesh_as_bvh", lua_builder_add_read_ply_mesh_as_bvh},
      {"add_ply_mesh", lua_builder_add_ply_mesh},
      {"add_ply_mesh_as_bvh", lua_builder_add_ply_mesh_as_bvh},
      {"add_voxel_grid", lua_builder_add_voxel_grid},
      {"make_triangle", lua_builder_make_triangle},
      {0, 0},
    };

    lua_register_type(l, SCENE_TNAME, scene_methods);
    lua_register_type(l, PRIMITIVE_TNAME, primitive_methods);
    lua_register_type(l, BUILDER_TNAME, builder_methods);
    luaL_newlib(l, builder_funs);
    return 1;
  }

  int lua_builder_make(lua_State* l) {
    lua_push_builder(l, std::make_shared<Builder>());
    return 1;
  }

  int lua_builder_build_scene(lua_State* l) {
    auto builder = lua_check_builder(l, 1);

    if(!builder->frame_stack.empty()) {
      return luaL_error(l, "Frame stack is not empty");
    }
    if(!builder->state_stack.empty()) {
      return luaL_error(l, "State stack is not empty");
    }

    auto scene = std::make_shared<Scene>();
    scene->primitive = lua_make_aggregate(*lua_get_ctx(l),
        builder->state, std::move(builder->frame));
    scene->bounds = scene->primitive->bounds();
    scene->centroid = scene->bounds.centroid();
    scene->radius = scene->bounds.radius();
    scene->lights = std::move(builder->lights);
    scene->default_camera = std::move(builder->camera);

    std::move(builder->meshes.begin(), builder->meshes.end(),
        std::back_inserter(scene->meshes));
    std::move(builder->prim_meshes.begin(), builder->prim_meshes.end(),
        std::back_inserter(scene->prim_meshes));

    lua_push_scene(l, std::move(scene));
    *builder = Builder();
    return 1;
  }

  int lua_builder_push_state(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    builder->state_stack.push_back(builder->state);
    return 0;
  }

  int lua_builder_pop_state(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    if(builder->state_stack.empty()) {
      return luaL_error(l, "State stack is empty (not balanced)");
    }
    builder->state = builder->state_stack.back();
    builder->state_stack.pop_back();
    return 0;
  }

  int lua_builder_push_frame(lua_State* l) {
    auto builder = lua_check_builder(l, 1);

    BuilderFrame new_frame;
    BuilderState new_state;
    new_state.material = builder->state.material;

    builder->state_stack.push_back(std::move(builder->state));
    builder->frame_stack.push_back(std::move(builder->frame));
    builder->state = std::move(new_state);
    builder->frame = std::move(new_frame);
    return 0;
  }

  int lua_builder_pop_frame(lua_State* l) {
    auto builder = lua_check_builder(l, 1);

    if(builder->frame_stack.empty()) {
      return luaL_error(l, "Frame stack is empty (not balanced)");
    } else if(builder->state_stack.empty()) {
      return luaL_error(l, "State stack is empty (not balanced)");
    }

    auto primitive = lua_make_aggregate(*lua_get_ctx(l),
        builder->state, std::move(builder->frame));
    builder->frame = std::move(builder->frame_stack.back());
    builder->state = std::move(builder->state_stack.back());
    builder->frame_stack.pop_back();
    builder->state_stack.pop_back();

    lua_push_primitive(l, std::shared_ptr<Primitive>(std::move(primitive)));
    return 1;
  }

  int lua_builder_set_transform(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    const Transform& trans = lua_check_transform(l, 2);
    builder->state.local_to_frame = builder->state.local_to_frame * trans;
    return 0;
  }

  int lua_builder_get_transform(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    lua_push_transform(l, builder->state.local_to_frame);
    return 1;
  }

  int lua_builder_set_material(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto material = lua_check_material(l, 2);
    builder->state.material = material;
    return 0;
  }

  int lua_builder_get_material(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    lua_push_material(l, builder->state.material);
    return 1;
  }

  int lua_builder_set_camera(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto camera = lua_check_camera(l, 2);
    builder->camera = camera;
    return 0;
  }

  int lua_builder_set_option(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    std::string option = luaL_checkstring(l, 2);
    if(option == "bvh split method") {
      std::string method = luaL_checkstring(l, 3);
      if(method == "sah") {
        builder->state.bvh_opts.split_method = BvhSplitMethod::Sah;
      } else if(method == "middle") {
        builder->state.bvh_opts.split_method = BvhSplitMethod::Middle;
      } else {
        luaL_error(l, "unknown bvh split method: %s", method.c_str());
      }
    } else if(option == "bvh leaf size") {
      builder->state.bvh_opts.leaf_size = luaL_checkinteger(l, 3);
    } else if(option == "bvh max leaf size") {
      builder->state.bvh_opts.max_leaf_size = luaL_checkinteger(l, 3);
    } else {
      luaL_error(l, "unknown option: %s", option.c_str());
    }
    return 0;
  }

  int lua_builder_add_shape(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto shape = lua_check_shape(l, 2);
    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;

    if(!material) {
      return luaL_error(l, "no material is set");
    }

    std::shared_ptr<Light> area_light;
    if(lua_gettop(l) >= 3) {
      area_light = lua_check_light(l, 3);
      if(!(area_light->flags & LIGHT_AREA)) {
        return luaL_error(l, "the light must be an area light");
      }
    }

    auto prim = std::make_unique<ShapePrimitive>(shape, material, area_light, transform);
    builder->frame.prims.push_back(std::move(prim));
    return 0;
  }

  int lua_builder_add_primitive(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto primitive = lua_check_primitive(l, 2);
    auto transform = builder->state.local_to_frame;
    auto frame_prim = std::make_unique<FramePrimitive>(transform, primitive);
    builder->frame.prims.push_back(std::move(frame_prim));
    return 0;
  }

  int lua_builder_add_triangle(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto mesh = lua_check_mesh(l, 2);
    uint32_t index = luaL_checkinteger(l, 3);

    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    std::unique_ptr<Primitive> prim;
    if(transform == identity()) {
      prim = std::make_unique<TriangleShapePrimitive>(mesh.get(), index, material);
    } else {
      prim = std::make_unique<ShapePrimitive>(
           std::make_shared<TriangleShape>(mesh.get(), index),
           material, nullptr, transform);
    }

    builder->frame.prims.push_back(std::move(prim));
    builder->meshes.insert(mesh);
    return 0;
  }

  int lua_builder_add_light(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    if(!builder->frame_stack.empty()) {
      luaL_error(l, "lights can only be added in the root frame");
      return 0;
    }
    builder->lights.push_back(lua_check_light(l, 2));
    return 0;
  }

  int lua_builder_add_read_ply_mesh(lua_State* l) {
    auto builder = lua_check_builder(l, 1);

    const char* file_name = luaL_checkstring(l, 2);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;
    if(!material) {
      luaL_error(l, "no material is set");
      return 0;
    }

    auto prim_mesh = std::make_shared<PrimitiveMesh>();
    prim_mesh->material = material;
    bool ok = read_ply_to_mesh(file, transform, prim_mesh->mesh,
      [&](uint32_t index) {
        builder->frame.prims.push_back(
          std::make_unique<MeshTrianglePrimitive>(prim_mesh.get(), index));
      });
    std::fclose(file);

    if(!ok) {
      return luaL_error(l, "Could not read ply file: %s", file_name);
    }

    builder->prim_meshes.insert(prim_mesh);
    return 0;
  }

  int lua_builder_add_read_ply_mesh_as_bvh(lua_State* l) {
    auto builder = lua_check_builder(l, 1);

    const char* file_name = luaL_checkstring(l, 2);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;
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

    builder->frame.prims.push_back(std::make_unique<MeshBvhPrimitive>(
        mesh.get(), material, std::move(indices), builder->state.bvh_opts,
        *lua_get_ctx(l)->pool));
    builder->meshes.insert(mesh);
    return 0;
  }

  int lua_builder_add_ply_mesh(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto ply_mesh = lua_check_ply_mesh(l, 2);

    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;
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
      builder->frame.prims.push_back(
          std::make_unique<MeshTrianglePrimitive>(prim_mesh.get(), t * 3));
    }
    builder->prim_meshes.insert(prim_mesh);
    return 0;
  }

  int lua_builder_add_ply_mesh_as_bvh(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto ply_mesh = lua_check_ply_mesh(l, 2);

    auto material = builder->state.material;
    auto transform = builder->state.local_to_frame;
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

    builder->frame.prims.push_back(std::make_unique<MeshBvhPrimitive>(
        mesh.get(), material, std::move(indices), builder->state.bvh_opts,
        *lua_get_ctx(l)->pool));
    builder->meshes.insert(mesh);
    return 0;
  }

  int lua_builder_add_voxel_grid(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto transform = builder->state.local_to_frame;

    int p = 2;
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

    builder->frame.prims.push_back(std::make_unique<VoxelGridPrimitive>(
        box, *grid, transform, std::move(voxel_materials)));
    return 0;
  }

  int lua_builder_make_triangle(lua_State* l) {
    auto builder = lua_check_builder(l, 1);
    auto mesh = lua_check_mesh(l, 2);
    uint32_t index = luaL_checkinteger(l, 3);

    builder->meshes.insert(mesh);
    lua_push_shape(l, std::make_shared<TriangleShape>(mesh.get(), index));
    return 1;
  }

  int lua_builder_get_scene_default_camera(lua_State* l) {
    auto scene = lua_check_scene(l, 1);
    lua_push_camera(l, scene->default_camera);
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


  std::shared_ptr<Builder> lua_check_builder(lua_State* l, int idx) {
    return lua_check_shared_obj<Builder, BUILDER_TNAME>(l, idx);
  }
  bool lua_test_builder(lua_State* l, int idx) {
    return lua_test_shared_obj<Builder, BUILDER_TNAME>(l, idx);
  }
  void lua_push_builder(lua_State* l, std::shared_ptr<Builder> builder) {
    lua_push_shared_obj<Builder, BUILDER_TNAME>(l, builder);
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

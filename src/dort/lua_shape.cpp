#include "dort/cube_shape.hpp"
#include "dort/disk_shape.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_shape.hpp"
#include "dort/mesh.hpp"
#include "dort/ply_mesh.hpp"
#include "dort/sphere_shape.hpp"
#include "dort/transform.hpp"
#include "dort/triangle_shape.hpp"

namespace dort {
  int lua_open_shape(lua_State* l) {
    const luaL_Reg shape_methods[] = {
      {"__gc", lua_gc_shared_obj<Shape, SHAPE_TNAME>},
      {0, 0},
    };

    const luaL_Reg mesh_methods[] = {
      {"__gc", lua_gc_shared_obj<Mesh, MESH_TNAME>},
      {0, 0},
    };

    const luaL_Reg ply_mesh_methods[] = {
      {"__gc", lua_gc_shared_obj<PlyMesh, PLY_MESH_TNAME>},
      {0, 0},
    };

    const luaL_Reg shape_funs[] = {
      {"make_sphere", lua_shape_make_sphere},
      {"make_disk", lua_shape_make_disk},
      {"make_cube", lua_shape_make_cube},
      {"make_mesh", lua_shape_make_mesh},
      {"read_ply_mesh", lua_ply_mesh_read},
      {0, 0},
    };

    lua_register_type(l, SHAPE_TNAME, shape_methods);
    lua_register_type(l, MESH_TNAME, mesh_methods);
    lua_register_type(l, PLY_MESH_TNAME, ply_mesh_methods);
    luaL_newlib(l, shape_funs);
    return 1;
  }

  int lua_shape_make_sphere(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<SphereShape>(radius));
    return 1;
  }

  int lua_shape_make_disk(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    float z = lua_param_float_opt(l, p, "z", 0.f);
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<DiskShape>(radius, z));
    return 1;
  }

  int lua_shape_make_cube(lua_State* l) {
    static const std::shared_ptr<CubeShape> CUBE_SHAPE =
      std::make_shared<CubeShape>();
    lua_push_shape(l, CUBE_SHAPE);
    return 1;
  }

  int lua_shape_make_mesh(lua_State* l) {
    Transform transform; {
      lua_getfield(l, 1, "transform");
      transform = lua_check_transform(l, -1);
      lua_pop(l, 1);
    }

    std::vector<Point> points; {
      lua_getfield(l, 1, "points");
      uint32_t point_count = lua_rawlen(l, -1);
      for(uint32_t i = 1; i <= point_count; ++i) {
        lua_rawgeti(l, -1, i);
        points.push_back(transform.apply(lua_check_point(l, -1)));
        lua_pop(l, 1);
      }
      lua_pushnil(l);
      lua_setfield(l, 1, "points");
      lua_pop(l, 1);
    }

    std::vector<uint32_t> vertices; {
      lua_getfield(l, 1, "vertices");
      uint32_t vertex_count = lua_rawlen(l, -1);
      for(uint32_t i = 1; i <= vertex_count; ++i) {
        lua_rawgeti(l, -1, i);
        vertices.push_back(lua_tointeger(l, -1));
        lua_pop(l, 1);
      }
      lua_pushnil(l);
      lua_setfield(l, 1, "vertices");
      lua_pop(l, 1);
    }

    lua_params_check_unused(l, 1);

    auto mesh = std::make_shared<Mesh>();
    mesh->points = std::move(points);
    mesh->vertices = std::move(vertices);
    lua_push_mesh(l, mesh);
    return 1;
  }

  int lua_ply_mesh_read(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open ply mesh file for reading: %s", file_name);
    }

    auto ply_mesh = std::make_shared<PlyMesh>();
    bool ok = read_ply_to_ply_mesh(file, *ply_mesh);
    std::fclose(file);

    if(!ok) {
      luaL_error(l, "Could not read ply file: %s", file_name);
    }

    lua_push_ply_mesh(l, ply_mesh);
    return 1;
  }


  std::shared_ptr<Shape> lua_check_shape(lua_State* l, int idx) {
    return lua_check_shared_obj<Shape, SHAPE_TNAME>(l, idx);
  }
  bool lua_test_shape(lua_State* l, int idx) {
    return lua_test_shared_obj<Shape, SHAPE_TNAME>(l, idx);
  }
  void lua_push_shape(lua_State* l, std::shared_ptr<Shape> shape) {
    lua_push_shared_obj<Shape, SHAPE_TNAME>(l, shape);
  }

  std::shared_ptr<Mesh> lua_check_mesh(lua_State* l, int idx) {
    return lua_check_shared_obj<Mesh, MESH_TNAME>(l, idx);
  }
  bool lua_test_mesh(lua_State* l, int idx) {
    return lua_test_shared_obj<Mesh, MESH_TNAME>(l, idx);
  }
  void lua_push_mesh(lua_State* l, std::shared_ptr<Mesh> mesh) {
    lua_push_shared_obj<Mesh, MESH_TNAME>(l, mesh);
  }

  std::shared_ptr<PlyMesh> lua_check_ply_mesh(lua_State* l, int idx) {
    return lua_check_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, idx);
  }
  bool lua_test_ply_mesh(lua_State* l, int idx) {
    return lua_test_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, idx);
  }
  void lua_push_ply_mesh(lua_State* l, std::shared_ptr<PlyMesh> ply_mesh) {
    lua_push_shared_obj<PlyMesh, PLY_MESH_TNAME>(l, ply_mesh);
  }
}

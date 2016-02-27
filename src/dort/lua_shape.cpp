#include "dort/disk.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_shape.hpp"
#include "dort/mesh.hpp"
#include "dort/sphere.hpp"
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

    lua_register_type(l, SHAPE_TNAME, shape_methods);
    lua_register_type(l, MESH_TNAME, mesh_methods);
    lua_register(l, "sphere", lua_shape_make_sphere);
    lua_register(l, "disk", lua_shape_make_disk);
    lua_register(l, "triangle", lua_shape_make_triangle);
    lua_register(l, "mesh", lua_shape_make_mesh);
    return 0;
  }

  int lua_shape_make_sphere(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<Sphere>(radius));
    return 1;
  }

  int lua_shape_make_disk(lua_State* l) {
    int p = 1;
    float radius = lua_param_float(l, p, "radius");
    float z = lua_param_float_opt(l, p, "z", 0.f);
    lua_params_check_unused(l, p);

    lua_push_shape(l, std::make_shared<Disk>(radius, z));
    return 1;
  }

  int lua_shape_make_triangle(lua_State* l) {
    auto mesh = lua_check_mesh(l, 1);
    uint32_t index = luaL_checkinteger(l, 2);
    lua_register_mesh(l, mesh);
    lua_push_shape(l, std::make_shared<TriangleShape>(mesh.get(), index));
    return 1;
  }

  int lua_shape_make_mesh(lua_State* l) {
    Builder& builder = lua_get_current_builder(l);
    auto& transform = builder.state.local_to_frame;

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
}

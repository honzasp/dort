#include "dort/lua_geometry.hpp"
#include "dort/geometry.hpp"

namespace dort {
  int lua_open_geometry(lua_State* l) {
    const luaL_Reg vector_methods[] = {
      {"x", lua_vector_get_x},
      {"y", lua_vector_get_y},
      {"z", lua_vector_get_z},
      {"__tostring", lua_vector_tostring},
      {"__add", lua_vector_add},
      {0, 0},
    };

    const luaL_Reg point_methods[] = {
      {"x", lua_point_get_x},
      {"y", lua_point_get_y},
      {"z", lua_point_get_z},
      {"__tostring", lua_point_tostring},
      {0, 0},
    };

    luaL_newmetatable(l, VECTOR_TNAME);
    lua_pushstring(l, "__index");
    lua_pushvalue(l, -2);
    lua_settable(l, -3);
    luaL_setfuncs(l, vector_methods, 0);
    lua_pop(l, 1);

    luaL_newmetatable(l, POINT_TNAME);
    lua_pushstring(l, "__index");
    lua_pushvalue(l, -2);
    lua_settable(l, -3);
    luaL_setfuncs(l, point_methods, 0);
    lua_pop(l, 1);

    lua_register(l, "vector", lua_vector_make);
    lua_register(l, "vector_x", lua_vector_get_x);
    lua_register(l, "vector_y", lua_vector_get_y);
    lua_register(l, "vector_z", lua_vector_get_z);
    lua_register(l, "point", lua_point_make);
    lua_register(l, "point_x", lua_point_get_x);
    lua_register(l, "point_y", lua_point_get_y);
    lua_register(l, "point_z", lua_point_get_z);
    return 0;
  }

  int lua_vector_make(lua_State* l) {
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    float z = luaL_checknumber(l, 3);
    lua_push_vector(l, Vector(x, y, z));
    return 1;
  }
  int lua_vector_get_x(lua_State* l) {
    lua_pushnumber(l, lua_get_vector(l, 1).v.x);
    return 1;
  }
  int lua_vector_get_y(lua_State* l) {
    lua_pushnumber(l, lua_get_vector(l, 1).v.y);
    return 1;
  }
  int lua_vector_get_z(lua_State* l) {
    lua_pushnumber(l, lua_get_vector(l, 1).v.z);
    return 1;
  }
  int lua_vector_tostring(lua_State* l) {
    Vector& vec = lua_get_vector(l, 1);
    lua_pushfstring(l, "vector(%f, %f, %f)", vec.v.x, vec.v.y, vec.v.z);
    return 1;
  }
  int lua_vector_add(lua_State* l) {
    lua_push_vector(l, lua_get_vector(l, 1) + lua_get_vector(l, 2));
    return 1;
  }

  int lua_point_make(lua_State* l) {
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    float z = luaL_checknumber(l, 3);
    lua_push_point(l, Point(x, y, z));
    return 1;
  }
  int lua_point_get_x(lua_State* l) {
    lua_pushnumber(l, lua_get_point(l, 1).v.x);
    return 1;
  }
  int lua_point_get_y(lua_State* l) {
    lua_pushnumber(l, lua_get_point(l, 1).v.y);
    return 1;
  }
  int lua_point_get_z(lua_State* l) {
    lua_pushnumber(l, lua_get_point(l, 1).v.z);
    return 1;
  }
  int lua_point_tostring(lua_State* l) {
    Point& pt = lua_get_point(l, 1);
    lua_pushfstring(l, "point(%f, %f, %f)", pt.v.x, pt.v.y, pt.v.z);
    return 1;
  }

  Vector& lua_get_vector(lua_State* l, int idx) {
    return *((Vector*)luaL_checkudata(l, idx, VECTOR_TNAME));
  }
  void lua_push_vector(lua_State* l, const Vector& vec) {
    static_assert(std::is_trivially_destructible<Vector>::value,
        "Lua vectors need to define finalizers");
    Vector* lua_vec = (Vector*)lua_newuserdata(l, sizeof(Vector));
    new (lua_vec) Vector(vec);
    luaL_getmetatable(l, VECTOR_TNAME);
    lua_setmetatable(l, -2);
  }

  Point& lua_get_point(lua_State* l, int idx) {
    return *((Point*)luaL_checkudata(l, idx, POINT_TNAME));
  }
  void lua_push_point(lua_State* l, const Point& pt) {
    static_assert(std::is_trivially_destructible<Point>::value,
        "Lua points need to define finalizers");
    Point* lua_pt = (Point*)lua_newuserdata(l, sizeof(Point));
    new (lua_pt) Point(pt);
    luaL_getmetatable(l, POINT_TNAME);
    lua_setmetatable(l, -2);
  }
}

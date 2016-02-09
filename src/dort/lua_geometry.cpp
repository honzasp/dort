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

    const luaL_Reg transform_methods[] = {
      {"apply", lua_transform_apply},
      {"apply_inv", lua_transform_apply_inv},
      {"inverse", lua_transform_inverse},
      {"__call", lua_transform_apply},
      {"__mul", lua_transform_mul},
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

    luaL_newmetatable(l, TRANSFORM_TNAME);
    lua_pushstring(l, "__index");
    lua_pushvalue(l, -2);
    lua_settable(l, -3);
    luaL_setfuncs(l, transform_methods, 0);
    lua_pop(l, 1);

    lua_register(l, "vector", lua_vector_make);
    lua_register(l, "point", lua_point_make);
    lua_register(l, "identity", lua_transform_identity);
    lua_register(l, "translate", lua_transform_translate);
    lua_register(l, "scale", lua_transform_scale);
    lua_register(l, "rotate_x", lua_transform_rotate_x);
    lua_register(l, "rotate_y", lua_transform_rotate_y);
    lua_register(l, "rotate_z", lua_transform_rotate_z);

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
    lua_pushnumber(l, lua_check_vector(l, 1).v.x);
    return 1;
  }
  int lua_vector_get_y(lua_State* l) {
    lua_pushnumber(l, lua_check_vector(l, 1).v.y);
    return 1;
  }
  int lua_vector_get_z(lua_State* l) {
    lua_pushnumber(l, lua_check_vector(l, 1).v.z);
    return 1;
  }
  int lua_vector_tostring(lua_State* l) {
    const Vector& vec = lua_check_vector(l, 1);
    lua_pushfstring(l, "vector(%f, %f, %f)", vec.v.x, vec.v.y, vec.v.z);
    return 1;
  }
  int lua_vector_add(lua_State* l) {
    lua_push_vector(l, lua_check_vector(l, 1) + lua_check_vector(l, 2));
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
    lua_pushnumber(l, lua_check_point(l, 1).v.x);
    return 1;
  }
  int lua_point_get_y(lua_State* l) {
    lua_pushnumber(l, lua_check_point(l, 1).v.y);
    return 1;
  }
  int lua_point_get_z(lua_State* l) {
    lua_pushnumber(l, lua_check_point(l, 1).v.z);
    return 1;
  }
  int lua_point_tostring(lua_State* l) {
    const Point& pt = lua_check_point(l, 1);
    lua_pushfstring(l, "point(%f, %f, %f)", pt.v.x, pt.v.y, pt.v.z);
    return 1;
  }

  int lua_transform_identity(lua_State* l) {
    lua_push_transform(l, identity());
    return 1;
  }
  int lua_transform_translate(lua_State* l) {
    if(lua_gettop(l) == 1) {
      lua_push_transform(l, translate(lua_check_vector(l, 1)));
    } else {
      lua_push_transform(l, translate(
            luaL_checknumber(l, 1),
            luaL_checknumber(l, 2),
            luaL_checknumber(l, 3)));
    }
    return 1;
  }
  int lua_transform_scale(lua_State* l) {
    if(lua_gettop(l) == 1) {
      lua_push_transform(l, scale(
            luaL_checknumber(l, 1)));
    } else {
      lua_push_transform(l, scale(
            luaL_checknumber(l, 1),
            luaL_checknumber(l, 2),
            luaL_checknumber(l, 3)));
    }
    return 1;
  }
  int lua_transform_rotate_x(lua_State* l) {
    lua_push_transform(l, rotate_x(luaL_checknumber(l, 1)));
    return 1;
  }
  int lua_transform_rotate_y(lua_State* l) {
    lua_push_transform(l, rotate_y(luaL_checknumber(l, 1)));
    return 1;
  }
  int lua_transform_rotate_z(lua_State* l) {
    lua_push_transform(l, rotate_z(luaL_checknumber(l, 1)));
    return 1;
  }

  int lua_transform_apply(lua_State* l) {
    const Transform& trans = lua_check_transform(l, 1);
    bool inv = false;
    int arg_idx = 2;
    if(lua_isboolean(l, 2)) {
      inv = lua_toboolean(l, 2);
      arg_idx = 3;
    }

    if(lua_test_point(l, arg_idx)) {
      lua_push_point(l, trans.apply(inv, lua_check_point(l, arg_idx)));
    } else if(lua_test_vector(l, arg_idx)) {
      lua_push_vector(l, trans.apply(inv, lua_check_vector(l, arg_idx)));
    } else {
      luaL_argerror(l, arg_idx, "Expected point or vector");
    }
    return 1;
  }
  int lua_transform_apply_inv(lua_State* l) {
    lua_pushboolean(l, true);
    lua_insert(l, 2);
    return lua_transform_apply(l);
  }
  int lua_transform_inverse(lua_State* l) {
    lua_push_transform(l, lua_check_transform(l, 1).inverse());
    return 1;
  }
  int lua_transform_mul(lua_State* l) {
    lua_push_transform(l,
        lua_check_transform(l, 1) *
        lua_check_transform(l, 2));
    return 1;
  }

  const Vector& lua_check_vector(lua_State* l, int idx) {
    return *((Vector*)luaL_checkudata(l, idx, VECTOR_TNAME));
  }
  bool lua_test_vector(lua_State* l, int idx) {
    return !!luaL_testudata(l, idx, VECTOR_TNAME);
    }
  void lua_push_vector(lua_State* l, const Vector& vec) {
    static_assert(std::is_trivially_destructible<Vector>::value,
        "Lua vectors need to define finalizers");
    Vector* lua_vec = (Vector*)lua_newuserdata(l, sizeof(Vector));
    new (lua_vec) Vector(vec);
    luaL_getmetatable(l, VECTOR_TNAME);
    lua_setmetatable(l, -2);
  }

  const Point& lua_check_point(lua_State* l, int idx) {
    return *((Point*)luaL_checkudata(l, idx, POINT_TNAME));
  }
  bool lua_test_point(lua_State* l, int idx) {
    return !!luaL_testudata(l, idx, POINT_TNAME);
    }
  void lua_push_point(lua_State* l, const Point& pt) {
    static_assert(std::is_trivially_destructible<Point>::value,
        "Lua points need to define finalizers");
    Point* lua_pt = (Point*)lua_newuserdata(l, sizeof(Point));
    new (lua_pt) Point(pt);
    luaL_getmetatable(l, POINT_TNAME);
    lua_setmetatable(l, -2);
  }

  const Transform& lua_check_transform(lua_State* l, int idx) {
    return *((Transform*)luaL_checkudata(l, idx, TRANSFORM_TNAME));
  }
  void lua_push_transform(lua_State* l, const Transform& trans) {
    static_assert(std::is_trivially_destructible<Transform>::value,
        "Lua transforms need to define finalizers");
    Transform* lua_trans = (Transform*)lua_newuserdata(l, sizeof(Transform));
    new (lua_trans) Transform(trans);
    luaL_getmetatable(l, TRANSFORM_TNAME);
    lua_setmetatable(l, -2);
  }
}

#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/geometry.hpp"

namespace dort {
  int lua_open_geometry(lua_State* l) {
    const luaL_Reg vector_methods[] = {
      {"x", lua_vector_get_x},
      {"y", lua_vector_get_y},
      {"z", lua_vector_get_z},
      {"__tostring", lua_vector_tostring},
      {"__add", lua_vector_add},
      {"__eq", lua_vector_eq},
      {0, 0},
    };

    const luaL_Reg point_methods[] = {
      {"x", lua_point_get_x},
      {"y", lua_point_get_y},
      {"z", lua_point_get_z},
      {"__tostring", lua_point_tostring},
      {"__eq", lua_point_eq},
      {0, 0},
    };

    const luaL_Reg transform_methods[] = {
      {"apply", lua_transform_apply},
      {"apply_inv", lua_transform_apply_inv},
      {"inverse", lua_transform_inverse},
      {"__call", lua_transform_apply},
      {"__mul", lua_transform_mul},
      {"__eq", lua_transform_eq},
      {0, 0},
    };

    lua_register_type(l, VECTOR_TNAME, vector_methods);
    lua_register_type(l, POINT_TNAME, point_methods);
    lua_register_type(l, TRANSFORM_TNAME, transform_methods);

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
  int lua_vector_eq(lua_State* l) {
    if(lua_test_vector(l, 1) ^ lua_test_vector(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_vector(l, 1) == lua_check_vector(l, 2));
    }
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
  int lua_point_eq(lua_State* l) {
    if(lua_test_point(l, 1) ^ lua_test_point(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_point(l, 1) == lua_check_point(l, 2));
    }
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
  int lua_transform_eq(lua_State* l) {
    if(lua_test_transform(l, 1) ^ lua_test_transform(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_transform(l, 1) == lua_check_transform(l, 2));
    }
    return 1;
  }

  const Vector& lua_check_vector(lua_State* l, int idx) {
    return lua_check_managed_obj<Vector, VECTOR_TNAME>(l, idx);
  }
  bool lua_test_vector(lua_State* l, int idx) {
    return lua_test_managed_obj<Vector, VECTOR_TNAME>(l, idx);
  }
  void lua_push_vector(lua_State* l, const Vector& vec) {
    return lua_push_managed_obj<Vector, VECTOR_TNAME>(l, vec);
  }

  const Point& lua_check_point(lua_State* l, int idx) {
    return lua_check_managed_obj<Point, POINT_TNAME>(l, idx);
  }
  bool lua_test_point(lua_State* l, int idx) {
    return lua_test_managed_obj<Point, POINT_TNAME>(l, idx);
  }
  void lua_push_point(lua_State* l, const Point& pt) {
    return lua_push_managed_obj<Point, POINT_TNAME>(l, pt);
  }

  const Transform& lua_check_transform(lua_State* l, int idx) {
    return lua_check_managed_obj<Transform, TRANSFORM_TNAME>(l, idx);
  }
  bool lua_test_transform(lua_State* l, int idx) {
    return lua_test_managed_obj<Transform, TRANSFORM_TNAME>(l, idx);
  }
  void lua_push_transform(lua_State* l, const Transform& trans) {
    return lua_push_managed_obj<Transform, TRANSFORM_TNAME>(l, trans);
  }
}

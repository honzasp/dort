/// Vectors and transformations.
// Beside the documented methods and functions, the vector types support the
// following methods:
//
// - `__tostring` -- a human-readable representation of the type and the value
// - `__eq` -- component-wise equality, also compares the type (for example,
//   `point(1,2,3) ~= vector(1,2,3)`)
// - `__add`, `__sub` and `__mul` -- supported when appropriate, the type is
//   determined from the types of the arguments. Arithmetic operations between
//   values of different types are supported only when they are meaningful (for
//   example, `Point + Vector` produces a `Point`, while `Point + Normal` is
//   meaningless).
// - `x`, `y`, `z` return the corresponding components (of course, `z` is not
//   available for two-dimensional types).
//
// @module dort.geometry
#include "dort/box_i.hpp"
#include "dort/geometry.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/rect_i.hpp"
#include "dort/transform.hpp"
#include "dort/vec_3i.hpp"
#include "dort/vec_2i.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  int lua_open_geometry(lua_State* l) {
    const luaL_Reg vector_methods[] = {
      {"x", lua_vector_get_x},
      {"y", lua_vector_get_y},
      {"z", lua_vector_get_z},
      {"length", lua_vector_length},
      {"__tostring", lua_vector_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__mul", lua_geometry_mul},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg point_methods[] = {
      {"x", lua_point_get_x},
      {"y", lua_point_get_y},
      {"z", lua_point_get_z},
      {"__tostring", lua_point_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__mul", lua_geometry_mul},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg normal_methods[] = {
      {"x", lua_normal_get_x},
      {"y", lua_normal_get_y},
      {"z", lua_normal_get_z},
      {"__tostring", lua_normal_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__mul", lua_geometry_mul},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg transform_methods[] = {
      {"apply", lua_transform_apply},
      {"apply_inv", lua_transform_apply_inv},
      {"__tostring", lua_transform_tostring},
      {"__call", lua_transform_apply},
      {"__mul", lua_transform_mul},
      {"__eq", lua_transform_eq},
      {0, 0},
    };

    const luaL_Reg vec3i_methods[] = {
      {"x", lua_vec3i_get_x},
      {"y", lua_vec3i_get_y},
      {"z", lua_vec3i_get_z},
      {"__tostring", lua_vec3i_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg vec2i_methods[] = {
      {"x", lua_vec2i_get_x},
      {"y", lua_vec2i_get_y},
      {"__tostring", lua_vec2i_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg vec2_methods[] = {
      {"x", lua_vec2_get_x},
      {"y", lua_vec2_get_y},
      {"__tostring", lua_vec2_tostring},
      {"__add", lua_geometry_add},
      {"__sub", lua_geometry_sub},
      {"__eq", lua_geometry_eq},
      {0, 0},
    };

    const luaL_Reg boxi_methods[] = {
      {"min", lua_boxi_get_min},
      {"max", lua_boxi_get_max},
      {"contains", lua_boxi_contains},
      {"__tostring", lua_boxi_tostring},
      {0, 0},
    };

    const luaL_Reg recti_methods[] = {
      {"min", lua_recti_get_min},
      {"max", lua_recti_get_max},
      {"__tostring", lua_recti_tostring},
      {0, 0},
    };

    const luaL_Reg geometry_funs[] = {
      {"vector", lua_vector_make},
      {"point", lua_point_make},
      {"normal", lua_normal_make},
      {"cross", lua_vector_cross},
      {"identity", lua_transform_identity},
      {"inverse", lua_transform_inverse},
      {"translate", lua_transform_translate},
      {"scale", lua_transform_scale},
      {"rotate_x", lua_transform_rotate_x},
      {"rotate_y", lua_transform_rotate_y},
      {"rotate_z", lua_transform_rotate_z},
      {"look_at", lua_transform_look_at},
      {"stretch", lua_transform_stretch},
      {"vec3i", lua_vec3i_make},
      {"vec2i", lua_vec2i_make},
      {"vec2", lua_vec2_make},
      {"boxi", lua_boxi_make},
      {"recti", lua_recti_make},
      {0, 0},
    };

    lua_register_type(l, VECTOR_TNAME, vector_methods);
    lua_register_type(l, POINT_TNAME, point_methods);
    lua_register_type(l, NORMAL_TNAME, normal_methods);
    lua_register_type(l, TRANSFORM_TNAME, transform_methods);
    lua_register_type(l, VEC3I_TNAME, vec3i_methods);
    lua_register_type(l, VEC2I_TNAME, vec2i_methods);
    lua_register_type(l, VEC2_TNAME, vec2_methods);
    lua_register_type(l, BOXI_TNAME, boxi_methods);
    lua_register_type(l, RECTI_TNAME, recti_methods);
    luaL_newlib(l, geometry_funs);
    return 1;
  }

  int lua_geometry_add(lua_State* l) {
    if(lua_test_vector(l, 1) && lua_test_point(l, 2)) {
      lua_push_point(l, lua_check_vector(l, 1) + lua_check_point(l, 2));
    } else if(lua_test_point(l, 1) && lua_test_vector(l, 2)) {
      lua_push_point(l, lua_check_point(l, 1) + lua_check_vector(l, 2));
    } else if(lua_test_point(l, 1) && lua_test_point(l, 2)) {
      lua_push_point(l, lua_check_point(l, 1) + lua_check_point(l, 2));
    } else if(lua_test_vector(l, 1) && lua_test_vector(l, 2)) {
      lua_push_vector(l, lua_check_vector(l, 1) + lua_check_vector(l, 2));
    } else if(lua_test_normal(l, 1) && lua_test_normal(l, 2)) {
      lua_push_normal(l, lua_check_normal(l, 1) + lua_check_normal(l, 2));
    } else if(lua_test_vec3i(l, 1) && lua_test_vec3i(l, 2)) {
      lua_push_vec3i(l, lua_check_vec3i(l, 1) + lua_check_vec3i(l, 2));
    } else if(lua_test_vec2i(l, 1) && lua_test_vec2i(l, 2)) {
      lua_push_vec2i(l, lua_check_vec2i(l, 1) + lua_check_vec2i(l, 2));
    } else {
      luaL_error(l, "Wrong combination of arguments to +");
    }
    return 1;
  }
  int lua_geometry_sub(lua_State* l) {
    if(lua_test_point(l, 1) && lua_test_vector(l, 2)) {
      lua_push_point(l, lua_check_point(l, 1) - lua_check_vector(l, 2));
    } else if(lua_test_point(l, 1) && lua_test_point(l, 2)) {
      lua_push_vector(l, lua_check_point(l, 1) - lua_check_point(l, 2));
    } else if(lua_test_vector(l, 1) && lua_test_vector(l, 2)) {
      lua_push_vector(l, lua_check_vector(l, 1) - lua_check_vector(l, 2));
    } else if(lua_test_normal(l, 1) && lua_test_normal(l, 2)) {
      lua_push_normal(l, lua_check_normal(l, 1) - lua_check_normal(l, 2));
    } else if(lua_test_vec3i(l, 1) && lua_test_vec3i(l, 2)) {
      lua_push_vec3i(l, lua_check_vec3i(l, 1) - lua_check_vec3i(l, 2));
    } else if(lua_test_vec2i(l, 1) && lua_test_vec2i(l, 2)) {
      lua_push_vec2i(l, lua_check_vec2i(l, 1) - lua_check_vec2i(l, 2));
    } else {
      luaL_error(l, "Wrong combination of arguments to -");
    }
    return 1;
  }
  int lua_geometry_mul(lua_State* l) {
    if(lua_test_point(l, 1)) {
      lua_push_point(l, lua_check_point(l, 1) * luaL_checknumber(l, 2));
    } else if(lua_test_vector(l, 1)) {
      lua_push_vector(l, lua_check_vector(l, 1) * luaL_checknumber(l, 2));
    } else if(lua_test_normal(l, 1)) {
      lua_push_normal(l, lua_check_normal(l, 1) * luaL_checknumber(l, 2));
    } else {
      luaL_error(l, "Wrong combination of arguments to *");
    }
    return 1;
  }
  int lua_geometry_eq(lua_State* l) {
    if(lua_test_vector(l, 1) && lua_test_vector(l, 2)) {
      lua_pushboolean(l, lua_check_vector(l, 1) == lua_check_vector(l, 2));
    } else if(lua_test_point(l, 1) && lua_test_point(l, 2)) {
      lua_pushboolean(l, lua_check_point(l, 1) == lua_check_point(l, 2));
    } else if(lua_test_normal(l, 1) && lua_test_normal(l, 2)) {
      lua_pushboolean(l, lua_check_normal(l, 1) == lua_check_normal(l, 2));
    } else if(lua_test_vec3i(l, 1) && lua_test_vec3i(l, 2)) {
      lua_pushboolean(l, lua_check_vec3i(l, 1) == lua_check_vec3i(l, 2));
    } else if(lua_test_vec2i(l, 1) && lua_test_vec2i(l, 2)) {
      lua_pushboolean(l, lua_check_vec2i(l, 1) == lua_check_vec2i(l, 2));
    } else {
      lua_pushboolean(l, false);
    }
    return 1;
  }

  /// Make a `Vector`.
  // Called as `vector(x, y, z)` to initialize the three numeric components or
  // as `vector(v)`, where `v` is another three-dimensional vector type.
  // @function vector
  // @param ...
  // @within Constructors
  int lua_vector_make(lua_State* l) {
    if(lua_gettop(l) == 1) {
      if(lua_test_vector(l, 1)) {
        lua_pushvalue(l, 1);
        return 1;
      } else if(lua_test_point(l, 1)) {
        lua_push_vector(l, Vector(lua_check_point(l, 1).v));
        return 1;
      } else if(lua_test_vec3i(l, 1)) {
        lua_push_vector(l, Vector(Vec3(lua_check_vec3i(l, 1))));
        return 1;
      }
    }
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
    Vector vec = lua_check_vector(l, 1);
    lua_pushfstring(l, "vector(%f, %f, %f)", vec.v.x, vec.v.y, vec.v.z);
    return 1;
  }
  int lua_vector_length(lua_State* l) {
    Vector vec = lua_check_vector(l, 1);
    lua_pushnumber(l, length(vec));
    return 1;
  }
  int lua_vector_cross(lua_State* l) {
    Vector v1 = lua_check_vector(l, 1);
    Vector v2 = lua_check_vector(l, 2);
    lua_push_vector(l, cross(v1, v2));
    return 1;
  }

  /// Make a `Point`.
  // Called as `point(x, y, z)` to initialize the three components or as
  // `point(v)` to initialize the point from another three dimensional vector.
  // @function point
  // @param ...
  // @within Constructors
  int lua_point_make(lua_State* l) {
    if(lua_gettop(l) == 1) {
      if(lua_test_point(l, 1)) {
        lua_pushvalue(l, 1);
        return 1;
      } else if(lua_test_vector(l, 1)) {
        lua_push_point(l, Point(lua_check_vector(l, 1).v));
        return 1;
      } else if(lua_test_vec3i(l, 1)) {
        lua_push_point(l, Point(Vec3(lua_check_vec3i(l, 1))));
        return 1;
      }
    }
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
    Point p = lua_check_point(l, 1);
    lua_pushfstring(l, "point(%f, %f, %f)", p.v.x, p.v.y, p.v.y);
    return 1;
  }
  
  /// Make a `Normal`.
  // Called as `normal(x, y, z)` to initialize the three components or as
  // `normal(v)` to initialize the normal from another three dimensional vector.
  // @function normal
  // @param ...
  // @within Constructors
  int lua_normal_make(lua_State* l) {
    if(lua_gettop(l) == 1) {
      if(lua_test_normal(l, 1)) {
        lua_pushvalue(l, 1);
        return 1;
      } else if(lua_test_vector(l, 1)) {
        lua_push_normal(l, Normal(lua_check_vector(l, 1).v));
        return 1;
      } else if(lua_test_vec3i(l, 1)) {
        lua_push_normal(l, Normal(Vec3(lua_check_vec3i(l, 1))));
        return 1;
      }
    }
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    float z = luaL_checknumber(l, 3);
    lua_push_normal(l, Normal(x, y, z));
    return 1;
  }
  int lua_normal_get_x(lua_State* l) {
    lua_pushnumber(l, lua_check_normal(l, 1).v.x);
    return 1;
  }
  int lua_normal_get_y(lua_State* l) {
    lua_pushnumber(l, lua_check_normal(l, 1).v.y);
    return 1;
  }
  int lua_normal_get_z(lua_State* l) {
    lua_pushnumber(l, lua_check_normal(l, 1).v.z);
    return 1;
  }
  int lua_normal_tostring(lua_State* l) {
    Normal n = lua_check_normal(l, 1);
    lua_pushfstring(l, "normal(%f, %f, %f)", n.v.x, n.v.y, n.v.z);
    return 1;
  }

  /// Identity transform.
  // @function identity
  // @within Transform
  int lua_transform_identity(lua_State* l) {
    lua_push_transform(l, identity());
    return 1;
  }
  /// Translation transform.
  // Called either as `translate(x, y, z)` or as `translate(v)`, where `v` is a
  // `Vector`.
  // @function translate
  // @param ...
  // @within Transform
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
  /// Scale transform.
  // Called either as `scale(x, y, z)`, or as `scale(s)`, where the same scale
  // factor `s` is used in all three axes.
  // @function scale
  // @param ...
  // @within Transform
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
  /// Rotation by `angle` through the X axis.
  // @function rotate_x
  // @param angle
  // @within Transform
  int lua_transform_rotate_x(lua_State* l) {
    lua_push_transform(l, rotate_x(luaL_checknumber(l, 1)));
    return 1;
  }
  /// Rotation by `angle` through the Y axis.
  // @function rotate_y
  // @param angle
  // @within Transform
  int lua_transform_rotate_y(lua_State* l) {
    lua_push_transform(l, rotate_y(luaL_checknumber(l, 1)));
    return 1;
  }
  /// Rotation by `angle` through the Z axis.
  // @function rotate_z
  // @param angle
  // @within Transform
  int lua_transform_rotate_z(lua_State* l) {
    lua_push_transform(l, rotate_z(luaL_checknumber(l, 1)));
    return 1;
  }
  /// Look-at transform.
  // The transform has origin at the `Point` `eye`, rotated so that the Z axis
  // goes through the `Point` `look` and Y axis point in the direction of
  // `Vector` `up`.
  // @function look_at
  // @param eye
  // @param look
  // @param up
  // @within Transform
  int lua_transform_look_at(lua_State* l) {
    Point eye = lua_check_point(l, 1);
    Point look = lua_check_point(l, 2);
    Vector up = lua_check_vector(l, 3);
    lua_push_transform(l, look_at(eye, look, up));
    return 1;
  }
  /// Stretch transform.
  // Stretches the coordinate system so that points `(-1,-1,-1)` and `(1,1,1)`
  // are positioned at `p1` and `p2`, respectively.
  // @function stretch
  // @param p1
  // @param p2
  // @within Transform
  int lua_transform_stretch(lua_State* l) {
    Point p1 = lua_check_point(l, 1);
    Point p2 = lua_check_point(l, 2);
    Point center = 0.5f * (p2 + p1);
    Vector radius = 0.5f * (p2 - p1);
    Transform stretch = translate(Vector(center.v)) * scale(radius);
    lua_push_transform(l, stretch);
    return 1;
  }

  int lua_transform_tostring(lua_State* l) {
    const Transform& trans = lua_check_transform(l, 1);
    luaL_checkstack(l, 100, nullptr);
    uint32_t n = 0;

    lua_pushstring(l, "transform("); ++n;
    for(bool inv: {false, true}) {
      const auto& mat = trans.get_mat(inv);
      if(inv) {
        lua_pushstring(l, ", "); ++n;
      }
      lua_pushstring(l, "mat4x4("); ++n;
      for(uint32_t col = 0; col < 4; ++col) {
        if(col != 0) {
          lua_pushstring(l, ", "); ++n;
        }
        lua_pushfstring(l, "vec4(%f, %f, %f, %f)", 
            mat.cols[col][0], mat.cols[col][1],
            mat.cols[col][2], mat.cols[col][3]);
        ++n;
      }
      lua_pushstring(l, ")"); ++n;
    }
    lua_pushstring(l, ")"); ++n;

    lua_concat(l, n);
    return 1;
  }

  /// @type Transform

  /// Apply the transform.
  // Applies the transform to `x`, which can be `Point` or `Vector`. If
  // `inverse` is true, the inverse of the transform is applied. The function is
  // also available as the `__call` method, so that transforms can be called as
  // functions.
  // @function apply
  // @param x
  // @param[opt] inverse
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
  /// Apply the inverse transform.
  // Effectively calls `self:apply(x, true)`.
  // @function apply_inv
  // @param x
  int lua_transform_apply_inv(lua_State* l) {
    lua_pushboolean(l, true);
    lua_insert(l, 2);
    return lua_transform_apply(l);
  }
  int lua_transform_inverse(lua_State* l) {
    lua_push_transform(l, lua_check_transform(l, 1).inverse());
    return 1;
  }
  /// Compose two transforms.
  // @function __mul
  // @param other
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
  /// @section end

  /// Make a `Vec3i`.
  // @function vec3i
  // @param x
  // @param y
  // @param z
  // @within Constructors
  int lua_vec3i_make(lua_State* l) {
    int32_t x = luaL_checkinteger(l, 1);
    int32_t y = luaL_checkinteger(l, 2);
    int32_t z = luaL_checkinteger(l, 3);
    lua_push_vec3i(l, Vec3i(x, y, z));
    return 1;
  }
  int lua_vec3i_get_x(lua_State* l) {
    lua_pushinteger(l, lua_check_vec3i(l, 1).x);
    return 1;
  }
  int lua_vec3i_get_y(lua_State* l) {
    lua_pushinteger(l, lua_check_vec3i(l, 1).y);
    return 1;
  }
  int lua_vec3i_get_z(lua_State* l) {
    lua_pushinteger(l, lua_check_vec3i(l, 1).z);
    return 1;
  }
  int lua_vec3i_tostring(lua_State* l) {
    Vec3i vec = lua_check_vec3i(l, 1);
    lua_pushfstring(l, "vec3i(%d, %d, %d)", vec.x, vec.y, vec.z);
    return 1;
  }

  /// Make a `Vec2i`.
  // @function vec2i
  // @param x
  // @param y
  // @within Constructors
  int lua_vec2i_make(lua_State* l) {
    int32_t x = luaL_checkinteger(l, 1);
    int32_t y = luaL_checkinteger(l, 2);
    lua_push_vec2i(l, Vec2i(x, y));
    return 1;
  }
  int lua_vec2i_get_x(lua_State* l) {
    lua_pushinteger(l, lua_check_vec2i(l, 1).x);
    return 1;
  }
  int lua_vec2i_get_y(lua_State* l) {
    lua_pushinteger(l, lua_check_vec2i(l, 1).y);
    return 1;
  }
  int lua_vec2i_tostring(lua_State* l) {
    Vec2i vec = lua_check_vec2i(l, 1);
    lua_pushfstring(l, "vec2i(%d, %d)", vec.x, vec.y);
    return 1;
  }

  /// Make a `Vec2`.
  // @function vec2
  // @param x
  // @param y
  // @within Constructors
  int lua_vec2_make(lua_State* l) {
    float x = luaL_checknumber(l, 1);
    float y = luaL_checknumber(l, 2);
    lua_push_vec2(l, Vec2(x, y));
    return 1;
  }
  int lua_vec2_get_x(lua_State* l) {
    lua_pushnumber(l, lua_check_vec2(l, 1).x);
    return 1;
  }
  int lua_vec2_get_y(lua_State* l) {
    lua_pushnumber(l, lua_check_vec2(l, 1).y);
    return 1;
  }
  int lua_vec2_tostring(lua_State* l) {
    Vec2 vec = lua_check_vec2(l, 1);
    lua_pushfstring(l, "vec2(%f, %f)", vec.x, vec.y);
    return 1;
  }

  /// Make a `Boxi`.
  // Creates an axis-aligned box with integral bounds:
  //
  // - `boxi()` -- create an empty `Boxi`
  // - `boxi(min, max)`, where `min` and `max` are `Vec3i` -- create a `Boxi`
  //   spanned by the two vectors.
  // - `boxi(x1, y1, z1, x2, y2, z2)` -- create a `Boxi` that spans the given
  //   ranges of coordinates.
  //
  // @function boxi
  // @param ...
  // @within Constructors
  int lua_boxi_make(lua_State* l) {
    if(lua_gettop(l) == 0) {
      lua_push_boxi(l, Boxi());
    } else if(lua_gettop(l) == 2) {
      Vec3i min = lua_check_vec3i(l, 1);
      Vec3i max = lua_check_vec3i(l, 2);
      lua_push_boxi(l, Boxi(min, max));
    } else {
      int32_t x1 = luaL_checkinteger(l, 1);
      int32_t y1 = luaL_checkinteger(l, 2);
      int32_t z1 = luaL_checkinteger(l, 3);
      int32_t x2 = luaL_checkinteger(l, 4);
      int32_t y2 = luaL_checkinteger(l, 5);
      int32_t z2 = luaL_checkinteger(l, 6);
      lua_push_boxi(l, Boxi(Vec3i(x1, y1, z1), Vec3i(x2, y2, z2)));
    }
    return 1;
  }

  /// @type Boxi

  /// Get the lower bound as `Vec3i`.
  // @function min
  int lua_boxi_get_min(lua_State* l) {
    const Boxi& box = lua_check_boxi(l, 1);
    lua_push_vec3i(l, box.p_min);
    return 1;
  }
  /// Get the upper bound as `Vec3i`.
  // @function max
  int lua_boxi_get_max(lua_State* l) {
    const Boxi& box = lua_check_boxi(l, 1);
    lua_push_vec3i(l, box.p_max);
    return 1;
  }
  int lua_boxi_tostring(lua_State* l) {
    Boxi box = lua_check_boxi(l, 1);
    lua_pushfstring(l, "boxi(vec3i(%d, %d, %d), vec3i(%d, %d, %d))",
        box.p_min.x, box.p_min.y, box.p_min.z,
        box.p_max.x, box.p_max.y, box.p_max.z);
    return 1;
  }
  /// Test a `Vec3i` for inclusion in the box.
  // @function contains
  // @param v
  int lua_boxi_contains(lua_State* l) {
    const Boxi& box = lua_check_boxi(l, 1);
    const Vec3i& vec = lua_check_vec3i(l, 2);
    lua_pushboolean(l, box.contains(vec));
    return 1;
  }
  /// @section end

  /// Make a `Recti`.
  // Creates an axis-aligned rectangle with integral bounds:
  //
  // - `recti()` -- create an empty `Recti`
  // - `recti(min, max)`, where `min` and `max` are `Vec2i` -- create a `Recti`
  //   spanned by the two vectors.
  // - `recti(x1, y1, x2, y2)` -- create a `Recti` that spans the given ranges
  //   of coorinates.
  //
  // @function recti
  // @param ...
  // @within Constructors
  int lua_recti_make(lua_State* l) {
    if(lua_gettop(l) == 0) {
      lua_push_recti(l, Recti());
    } else if(lua_gettop(l) == 2) {
      Vec2i min = lua_check_vec2i(l, 1);
      Vec2i max = lua_check_vec2i(l, 2);
      lua_push_recti(l, Recti(min, max));
    } else {
      int32_t x1 = luaL_checkinteger(l, 1);
      int32_t y1 = luaL_checkinteger(l, 2);
      int32_t x2 = luaL_checkinteger(l, 3);
      int32_t y2 = luaL_checkinteger(l, 4);
      lua_push_recti(l, Recti(Vec2i(x1, y1), Vec2i(x2, y2)));
    }
    return 1;
  }

  /// @type Recti

  /// Get the lower bound as `Vec2i`.
  // @function min
  int lua_recti_get_min(lua_State* l) {
    const Recti& rect = lua_check_recti(l, 1);
    lua_push_vec2i(l, rect.p_min);
    return 1;
  }
  /// Get the upper bound as `Vec2i`.
  // @function max
  int lua_recti_get_max(lua_State* l) {
    const Recti& rect = lua_check_recti(l, 1);
    lua_push_vec2i(l, rect.p_max);
    return 1;
  }
  int lua_recti_tostring(lua_State* l) {
    Recti rect = lua_check_recti(l, 1);
    lua_pushfstring(l, "recti(vec2i(%d, %d), vec2i(%d, %d))",
        rect.p_min.x, rect.p_min.y, rect.p_max.x, rect.p_max.y);
    return 1;
  }
  /// @section end

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

  const Normal& lua_check_normal(lua_State* l, int idx) {
    return lua_check_managed_obj<Normal, NORMAL_TNAME>(l, idx);
  }
  bool lua_test_normal(lua_State* l, int idx) {
    return lua_test_managed_obj<Normal, NORMAL_TNAME>(l, idx);
  }
  void lua_push_normal(lua_State* l, const Normal& n) {
    return lua_push_managed_obj<Normal, NORMAL_TNAME>(l, n);
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

  const Vec3i& lua_check_vec3i(lua_State* l, int idx) {
    return lua_check_managed_obj<Vec3i, VEC3I_TNAME>(l, idx);
  }
  bool lua_test_vec3i(lua_State* l, int idx) {
    return lua_test_managed_obj<Vec3i, VEC3I_TNAME>(l, idx);
  }
  void lua_push_vec3i(lua_State* l, const Vec3i& vec) {
    return lua_push_managed_obj<Vec3i, VEC3I_TNAME>(l, vec);
  }

  const Vec2i& lua_check_vec2i(lua_State* l, int idx) {
    return lua_check_managed_obj<Vec2i, VEC2I_TNAME>(l, idx);
  }
  bool lua_test_vec2i(lua_State* l, int idx) {
    return lua_test_managed_obj<Vec2i, VEC2I_TNAME>(l, idx);
  }
  void lua_push_vec2i(lua_State* l, const Vec2i& vec) {
    return lua_push_managed_obj<Vec2i, VEC2I_TNAME>(l, vec);
  }

  const Vec2& lua_check_vec2(lua_State* l, int idx) {
    return lua_check_managed_obj<Vec2, VEC2_TNAME>(l, idx);
  }
  bool lua_test_vec2(lua_State* l, int idx) {
    return lua_test_managed_obj<Vec2, VEC2_TNAME>(l, idx);
  }
  void lua_push_vec2(lua_State* l, Vec2 vec) {
    return lua_push_managed_obj<Vec2, VEC2_TNAME>(l, vec);
  }

  const Boxi& lua_check_boxi(lua_State* l, int idx) {
    return lua_check_managed_obj<Boxi, BOXI_TNAME>(l, idx);
  }
  bool lua_test_boxi(lua_State* l, int idx) {
    return lua_test_managed_obj<Boxi, BOXI_TNAME>(l, idx);
  }
  void lua_push_boxi(lua_State* l, const Boxi& vec) {
    return lua_push_managed_obj<Boxi, BOXI_TNAME>(l, vec);
  }

  const Recti& lua_check_recti(lua_State* l, int idx) {
    return lua_check_managed_obj<Recti, RECTI_TNAME>(l, idx);
  }
  bool lua_test_recti(lua_State* l, int idx) {
    return lua_test_managed_obj<Recti, RECTI_TNAME>(l, idx);
  }
  void lua_push_recti(lua_State* l, const Recti& vec) {
    return lua_push_managed_obj<Recti, RECTI_TNAME>(l, vec);
  }
}

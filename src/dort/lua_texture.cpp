#include "dort/basic_texture_maps.hpp"
#include "dort/basic_textures.hpp"
#include "dort/image_texture.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_texture.hpp"

namespace dort {
  int lua_open_texture(lua_State* l) {
    const luaL_Reg texture_float_methods[] = {
      {"__eq", lua_texture_eq},
      {"__add", lua_texture_add},
      {"__mul", lua_texture_mul},
      {"__gc", lua_gc_shared_obj<Texture<float>, TEXTURE_FLOAT_TNAME>},
      {0, 0},
    };

    const luaL_Reg texture_spectrum_methods[] = {
      {"__eq", lua_texture_eq},
      {"__add", lua_texture_add},
      {"__mul", lua_texture_mul},
      {"__gc", lua_gc_shared_obj<Texture<Spectrum>, TEXTURE_SPECTRUM_TNAME>},
      {0, 0},
    };

    const luaL_Reg texture_map_2d_methods[] = {
      {"__eq", lua_texture_map_2d_eq},
      {"__gc", lua_gc_shared_obj<TextureMap2d, TEXTURE_MAP_2D_TNAME>},
      {0, 0},
    };

    lua_register_type(l, TEXTURE_FLOAT_TNAME, texture_float_methods);
    lua_register_type(l, TEXTURE_SPECTRUM_TNAME, texture_spectrum_methods);
    lua_register_type(l, TEXTURE_MAP_2D_TNAME, texture_map_2d_methods);

    lua_register(l, "const_texture", lua_texture_make_const);
    lua_register(l, "lerp_texture", lua_texture_make_lerp);
    lua_register(l, "checkerboard_texture", lua_texture_make_checkerboard);
    lua_register(l, "map_debug_texture", lua_texture_make_map_debug);
    lua_register(l, "image_texture", lua_texture_make_image);

    lua_register(l, "uv_texture_map", lua_texture_map_2d_make_uv);
    lua_register(l, "xy_texture_map", lua_texture_map_2d_make_xy);
    lua_register(l, "spherical_texture_map", lua_texture_map_2d_make_spherical);
    lua_register(l, "cylindrical_texture_map", lua_texture_map_2d_make_cylindrical);

    return 0;
  }

  int lua_texture_make_const(lua_State* l) {
    if(lua_test_spectrum(l, 1)) {
      lua_push_texture_spectrum(l, const_texture(lua_check_spectrum(l, 1)));
    } else if(lua_isnumber(l, 1)) {
      lua_push_texture_float(l, const_texture(lua_tonumber(l, 1)));
    } else {
      luaL_argerror(l, 1, "Expected a number or spectrum");
    }
    return 1;
  }

  int lua_texture_make_lerp(lua_State* l) {
    int p = 1;
    LuaTextureType type = lua_join_texture_types(
        lua_infer_texture_type_from_param(l, p, "tex_0"),
        lua_infer_texture_type_from_param(l, p, "tex_1"));
    if(type == LuaTextureType::Float) {
      lua_push_texture_float(l, lerp_texture(
            lua_param_texture_float(l, p, "t"),
            lua_param_texture_float(l, p, "tex_0"),
            lua_param_texture_float(l, p, "tex_1")));
    } else if(type == LuaTextureType::Spectrum) {
      lua_push_texture_spectrum(l, lerp_texture(
            lua_param_texture_float(l, p, "t"),
            lua_param_texture_spectrum(l, p, "tex_0"),
            lua_param_texture_spectrum(l, p, "tex_1")));
    } else {
      luaL_argerror(l, 1, 
          "Parameters tex_0 and tex_1 must be textures of the same type");
    }
    lua_params_check_unused(l, p);
    return 1;
  }

  int lua_texture_make_checkerboard(lua_State* l) {
    int p = 1;
    LuaTextureType type = lua_join_texture_types(
        lua_infer_texture_type_from_param(l, p, "even_check"),
        lua_infer_texture_type_from_param(l, p, "odd_check"));
    if(type == LuaTextureType::Float) {
      lua_push_texture_float(l, checkerboard_texture(
            lua_param_texture_map_2d_opt(l, p, "map", uv_texture_map_2d()),
            lua_param_float_opt(l, p, "check_size", 1.f),
            lua_param_float_opt(l, p, "even_check", 0.f),
            lua_param_float_opt(l, p, "odd_check", 1.f)));
    } else if(type == LuaTextureType::Spectrum) {
      lua_push_texture_spectrum(l, checkerboard_texture(
            lua_param_texture_map_2d_opt(l, p, "map", uv_texture_map_2d()),
            lua_param_float_opt(l, p, "check_size", 1.f),
            lua_param_spectrum_opt(l, p, "even_check", Spectrum(0.f)),
            lua_param_spectrum_opt(l, p, "odd_check", Spectrum(1.f))));
    } else {
      luaL_argerror(l, 1,
          "Parameters odd_check and even_check must be both floats or spectra");
    }
    lua_params_check_unused(l, p);
    return 1;
  }

  int lua_texture_make_map_debug(lua_State* l) {
    if(lua_gettop(l) == 0) {
      lua_push_texture_spectrum(l, map_debug_texture(uv_texture_map_2d()));
    } else {
      lua_push_texture_spectrum(l, map_debug_texture(
            lua_check_texture_map_2d(l, 1)));
    }
    return 1;
  }

  int lua_texture_make_image(lua_State* l) {
    int p = 1;
    lua_push_texture_spectrum(l, image_texture(
          lua_param_texture_map_2d_opt(l, p, "map", uv_texture_map_2d()),
          lua_param_image(l, p, "image")));
    lua_params_check_unused(l, p);
    return 1;
  }

  int lua_texture_eq(lua_State* l) {
    if(lua_test_texture_float(l, 1) && lua_test_texture_float(l, 2)) {
      lua_pushboolean(l, lua_check_texture_float(l, 1).get() ==
          lua_check_texture_float(l, 2).get());
    } else if(lua_test_texture_spectrum(l, 1) && lua_test_texture_spectrum(l, 2)) {
      lua_pushboolean(l, lua_check_texture_spectrum(l, 1).get() ==
          lua_check_texture_spectrum(l, 2).get());
    } else {
      lua_pushboolean(l, false);
    }
    return 1;
  }

  int lua_texture_add(lua_State* l) {
    if(lua_test_texture_float(l, 1) && lua_test_texture_float(l, 2)) {
      lua_push_texture_float(l, add_texture(
            lua_check_texture_float(l, 1), lua_check_texture_float(l, 2)));
    } else if(lua_test_texture_spectrum(l, 1) && lua_test_texture_spectrum(l, 2)) {
      lua_push_texture_spectrum(l, add_texture(
            lua_check_texture_spectrum(l, 1), lua_check_texture_spectrum(l, 2)));
    } else {
      luaL_error(l, "Only textures of the same type can be added");
    }
    return 1;
  }

  int lua_texture_mul(lua_State* l) {
    for(int scale_arg: {1, 2}) {
      int scaled_arg = 3 - scale_arg;
      if(lua_test_texture_float(l, scale_arg) || lua_isnumber(l, scale_arg)) {
        if(lua_test_texture_float(l, scaled_arg)) {
          lua_push_texture_float(l, scale_texture(
                lua_cast_texture_float(l, scale_arg),
                lua_cast_texture_float(l, scaled_arg)));
          return 1;
        } else if(lua_test_texture_spectrum(l, scaled_arg)) {
          lua_push_texture_spectrum(l, scale_texture(
                lua_cast_texture_float(l, scale_arg),
                lua_cast_texture_spectrum(l, scaled_arg)));
          return 1;
        }
      }
    }
    luaL_error(l, "Texture can be multiplied only by a number or float texture");
    return 1;
  }

  int lua_texture_map_2d_make_uv(lua_State* l) {
    lua_push_texture_map_2d(l, uv_texture_map_2d());
    return 1;
  }
  int lua_texture_map_2d_make_xy(lua_State* l) {
    Transform trans;
    if(lua_gettop(l) != 0) {
      trans = lua_check_transform(l, 1);
    }
    lua_push_texture_map_2d(l, xy_texture_map_2d(
          lua_current_frame_transform(l) * trans));
    return 1;
  }
  int lua_texture_map_2d_make_spherical(lua_State* l) {
    Transform trans;
    if(lua_gettop(l) != 0) {
      trans = lua_check_transform(l, 1);
    }
    lua_push_texture_map_2d(l, spherical_texture_map_2d(
          lua_current_frame_transform(l) * trans));
    return 1;
  }
  int lua_texture_map_2d_make_cylindrical(lua_State* l) {
    Transform trans;
    if(lua_gettop(l) != 0) {
      trans = lua_check_transform(l, 1);
    }
    lua_push_texture_map_2d(l, cylindrical_texture_map_2d(
          lua_current_frame_transform(l) * trans));
    return 1;
  }

  int lua_texture_map_2d_eq(lua_State* l) {
    if(!lua_test_texture_map_2d(l, 2)) {
      lua_pushboolean(l, false);
    } else {
      lua_pushboolean(l, lua_check_texture_map_2d(l, 1).get() ==
          lua_check_texture_map_2d(l, 2).get());
    }
    return 1;
  }



  std::shared_ptr<Texture<float>> lua_cast_texture_float(lua_State* l, int idx) {
    if(lua_isnumber(l, idx)) {
      return const_texture(lua_tonumber(l, idx));
    } else {
      return lua_check_texture_float(l, idx);
    }
  }
  std::shared_ptr<Texture<float>> lua_check_texture_float(lua_State* l, int idx) {
    return lua_check_shared_obj<Texture<float>, TEXTURE_FLOAT_TNAME>(l, idx);
  }
  bool lua_test_texture_float(lua_State* l, int idx) {
    return lua_test_shared_obj<Texture<float>, TEXTURE_FLOAT_TNAME>(l, idx);
  }
  void lua_push_texture_float(lua_State* l, std::shared_ptr<Texture<float>> tex) {
    return lua_push_shared_obj<Texture<float>, TEXTURE_FLOAT_TNAME>(l, tex);
  }

  std::shared_ptr<Texture<Spectrum>> lua_cast_texture_spectrum(lua_State* l, int idx) {
    if(lua_test_spectrum(l, idx)) {
      return const_texture(lua_check_spectrum(l, idx));
    } else {
      return lua_check_texture_spectrum(l, idx);
    }
  }
  std::shared_ptr<Texture<Spectrum>> lua_check_texture_spectrum(lua_State* l, int idx) {
    return lua_check_shared_obj<Texture<Spectrum>, TEXTURE_SPECTRUM_TNAME>(l, idx);
  }
  bool lua_test_texture_spectrum(lua_State* l, int idx) {
    return lua_test_shared_obj<Texture<Spectrum>, TEXTURE_SPECTRUM_TNAME>(l, idx);
  }
  void lua_push_texture_spectrum(lua_State* l, std::shared_ptr<Texture<Spectrum>> tex) {
    return lua_push_shared_obj<Texture<Spectrum>, TEXTURE_SPECTRUM_TNAME>(l, tex);
  }

  std::shared_ptr<TextureMap2d> lua_check_texture_map_2d(lua_State* l, int idx) {
    return lua_check_shared_obj<TextureMap2d, TEXTURE_MAP_2D_TNAME>(l, idx);
  }
  bool lua_test_texture_map_2d(lua_State* l, int idx) {
    return lua_test_shared_obj<TextureMap2d, TEXTURE_MAP_2D_TNAME>(l, idx);
  }
  void lua_push_texture_map_2d(lua_State* l, std::shared_ptr<TextureMap2d> map) {
    return lua_push_shared_obj<TextureMap2d, TEXTURE_MAP_2D_TNAME>(l, map);
  }

  LuaTextureType lua_join_texture_types(LuaTextureType t1, LuaTextureType t2) {
    if(t1 == t2) {
      return t1;
    } else if(t1 == LuaTextureType::Unknown) {
      return t2;
    } else if(t2 == LuaTextureType::Unknown) {
      return t1;
    } else {
      return LuaTextureType::Mismatch;
    }
  }

  LuaTextureType lua_infer_texture_type_from_param(lua_State* l,
      int params_idx, const char* param_name)
  {
    if(!lua_param_is_set(l, params_idx, param_name)) {
      return LuaTextureType::Unknown;
    } else if(lua_param_is_float(l, params_idx, param_name) ||
        lua_param_is_texture_float(l, params_idx, param_name)) {
      return LuaTextureType::Float;
    } else if(lua_param_is_spectrum(l, params_idx, param_name) ||
        lua_param_is_texture_spectrum(l, params_idx, param_name)) {
      return LuaTextureType::Spectrum;
    } else {
      return LuaTextureType::Mismatch;
    }
  }
}

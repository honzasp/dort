#include "dort/basic_textures.hpp"
#include "dort/image_texture.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_spectrum.hpp"
#include "dort/lua_texture.hpp"
#include "dort/lua_texture_magic.hpp"

namespace dort {
  int lua_open_texture(lua_State* l) {
    const luaL_Reg texture_methods[] = {
      {"__add", lua_texture_add},
      {"__mul", lua_texture_mul},
      {"__gc", lua_gc_managed_obj<LuaTexture, TEXTURE_TNAME>},
      {0, 0},
    };

    const luaL_Reg texture_funs[] = {
      {"make_const_geom", lua_texture_make_const<const DiffGeom&>},
      {"make_const_1d", lua_texture_make_const<float>},
      {"make_const_2d", lua_texture_make_const<Vec2>},
      {"make_const_3d", lua_texture_make_const<Vec3>},
      {"make_lerp", lua_texture_make_lerp},
      {"make_checkerboard_1d", lua_texture_make_checkerboard<float>},
      {"make_checkerboard_2d", lua_texture_make_checkerboard<Vec2>},
      {"make_checkerboard_3d", lua_texture_make_checkerboard<Vec3>},
      {"make_image", lua_texture_make_image},
      /*{"make_noise", lua_texture_make_noise},
      {"make_map_uv", lua_texture_map_make_uv},
      {"make_map_xy", lua_texture_map_make_xy},
      {"make_map_spherical", lua_texture_map_make_spherical},
      {"make_map_cylindrical", lua_texture_map_make_cylindrical},
      {"make_map_xyz", lua_texture_map_make_xyz},*/
      {0, 0},
    };

    lua_register_type(l, TEXTURE_TNAME, texture_methods);
    luaL_newlib(l, texture_funs);
    return 1;
  }

  template<class In> int lua_texture_make_const(lua_State* l) {
    LuaTexture lua_tex;
    lua_tex.in_type = lua_texture_in_v<In>();
    if(lua_test_spectrum(l, 1)) {
      lua_tex.out_type = LuaTextureOut::Spectrum;
      lua_tex.texture = const_texture<Spectrum, In>(lua_check_spectrum(l, 1));
    } else if(lua_isnumber(l, 1)) {
      lua_tex.out_type = LuaTextureOut::Float;
      lua_tex.texture = const_texture<float, In>(lua_tonumber(l, 1));
    } else {
      return luaL_argerror(l, 1, "Can only make textures from numbers or spectra");
    }
    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureLerpHandler {
    static void handle(LuaTexture& lua_tex, const LuaTexture& lua_tex_t,
        const LuaTexture& lua_tex_0, const LuaTexture& lua_tex_1)
    {
      auto tex_t = lua_tex_t.get<float, In>();
      auto tex_0 = lua_tex_0.get<Out, In>();
      auto tex_1 = lua_tex_1.get<Out, In>();
      lua_tex.texture = lerp_texture(tex_t, tex_0, tex_1);
    }
  };
  int lua_texture_make_lerp(lua_State* l) {
    int p = 1;
    LuaTexture lua_tex_0 = lua_param_texture(l, p, "tex_0");
    LuaTexture lua_tex_1 = lua_param_texture(l, p, "tex_1");
    LuaTexture lua_tex_t = lua_param_texture(l, p, "t");
    lua_params_check_unused(l, p);

    if(lua_tex_0.in_type != lua_tex_1.in_type) {
      return luaL_argerror(l, 1, 
          "The textures tex_0 and tex_1 must have the same input type");
    } else if(lua_tex_0.out_type != lua_tex_1.out_type) {
      return luaL_argerror(l, 1,
          "The textures tex_0 and tex_1 must have the same output type");
    } else if(lua_tex_t.out_type != LuaTextureOut::Float) {
      return luaL_argerror(l, 1,
          "The texture t must have float output type");
    } else if(lua_tex_t.in_type != lua_tex_0.in_type) {
      return luaL_argerror(l, 1,
          "The texture t must have the same input type as tex_0 and tex_1");
    }

    LuaTexture lua_tex;
    lua_tex.in_type = lua_tex_0.in_type;
    lua_tex.out_type = lua_tex_0.out_type;

    lua_texture_dispatch_out_in<LuaTextureLerpHandler>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_t, lua_tex_0, lua_tex_1);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class In>
  struct LuaTextureCheckerboardHandler {
    template<class Out>
    struct H {
      static void handle(LuaTexture& lua_tex, const LuaTexture& even_lua_tex,
          const LuaTexture& odd_lua_tex, float check_size)
      {
        auto even_tex = even_lua_tex.get<Out, Vec2>();
        auto odd_tex = odd_lua_tex.get<Out, Vec2>();
        lua_tex.texture = checkerboard_texture<Out, Vec2>(
            even_tex, odd_tex, check_size);
      }
    };
  };

  template<class In> int lua_texture_make_checkerboard(lua_State* l) {
    int p = 1;
    LuaTexture even_lua_tex = lua_param_texture(l, p, "even");
    LuaTexture odd_lua_tex = lua_param_texture(l, p, "odd");
    float check_size = lua_param_float_opt(l, p, "check_size", 1.f);

    if(even_lua_tex.in_type != lua_texture_in_v<In>()) {
      return luaL_argerror(l, 1, "Texture even must have compliant input type");
    } else if(odd_lua_tex.in_type != lua_texture_in_v<In>()) {
      return luaL_argerror(l, 1, "Texture odd must have compliant input type");
    } else if(even_lua_tex.out_type != odd_lua_tex.out_type) {
      return luaL_argerror(l, 1, "Textures even and odd "
          "must have the same output type");
    }

    LuaTexture lua_tex;
    lua_tex.in_type = lua_texture_in_v<In>();
    lua_tex.out_type = even_lua_tex.out_type;

    lua_texture_dispatch_out<LuaTextureCheckerboardHandler<In>::template H>(
        lua_tex.out_type,
        lua_tex, even_lua_tex, odd_lua_tex, check_size);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_make_image(lua_State* l) {
    int p = 1;
    auto image = lua_param_image(l, p, "image");
    lua_params_check_unused(l, p);

    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Vec2;
    lua_tex.out_type = LuaTextureOut::Spectrum;
    lua_tex.texture = std::make_shared<ImageTexture>(image);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureAddHandler {
    static void handle(LuaTexture& lua_tex,
        const LuaTexture& lua_tex_0, const LuaTexture& lua_tex_1)
    {
      auto tex_0 = lua_tex_0.get<Out, In>();
      auto tex_1 = lua_tex_1.get<Out, In>();
      lua_tex.texture = add_texture(tex_0, tex_1);
    }
  };
  int lua_texture_add(lua_State* l) {
    LuaTexture lua_tex_0 = lua_check_texture(l, 1);
    LuaTexture lua_tex_1 = lua_check_texture(l, 2);

    if(lua_tex_0.in_type != lua_tex_1.in_type) {
      return luaL_error(l, "Textures that are added must have the same input type");
    } else if(lua_tex_0.out_type != lua_tex_1.out_type) {
      return luaL_error(l, "Textures that are added must have the same output type");
    }

    LuaTexture lua_tex;
    lua_tex.in_type = lua_tex_0.in_type;
    lua_tex.out_type = lua_tex_0.out_type;

    lua_texture_dispatch_out_in<LuaTextureAddHandler>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_0, lua_tex_1);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureMulHandler {
    static void handle(LuaTexture& lua_tex,
        const LuaTexture& lua_tex_0, const LuaTexture& lua_tex_1)
    {
      auto tex_0 = lua_tex_0.get<Out, In>();
      auto tex_1 = lua_tex_1.get<Out, In>();
      lua_tex.texture = mul_texture(tex_0, tex_1);
    }
  };

  template<class Out, class In>
  struct LuaTextureScaleHandler {
    static void handle(LuaTexture& lua_tex,
        const LuaTexture& lua_tex_float, const LuaTexture& lua_tex_other)
    {
      auto tex_float = lua_tex_float.get<float, In>();
      auto tex_other = lua_tex_other.get<Out, In>();
      lua_tex.texture = scale_texture(tex_float, tex_other);
    }
  };


  int lua_texture_mul(lua_State* l) {
    LuaTexture lua_tex_0 = lua_check_texture(l, 1);
    LuaTexture lua_tex_1 = lua_check_texture(l, 2);

    LuaTexture lua_tex_float;
    LuaTexture lua_tex_other;

    if(lua_tex_0.in_type != lua_tex_1.in_type) {
      return luaL_error(l, "Textures that are added must have the same input type");
    } else if(lua_tex_0.out_type == LuaTextureOut::Float) {
      lua_tex_float = lua_tex_0;
      lua_tex_other = lua_tex_1;
    } else if(lua_tex_1.out_type == LuaTextureOut::Float) {
      lua_tex_float = lua_tex_1;
      lua_tex_other = lua_tex_0;
    } else if(lua_tex_0.out_type == lua_tex_1.out_type) {
      LuaTexture lua_tex;
      lua_tex.out_type = lua_tex_0.out_type;
      lua_tex.in_type = lua_tex_0.in_type;
      lua_texture_dispatch_out_in<LuaTextureMulHandler>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_0, lua_tex_1);
    } else {
      return luaL_error(l, "Incompatible textures to multiply");
    }

    LuaTexture lua_tex;
    lua_tex.out_type = lua_tex_other.out_type;
    lua_tex.in_type = lua_tex_other.in_type;
    lua_texture_dispatch_out_in<LuaTextureScaleHandler>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_float, lua_tex_other);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  const LuaTexture& lua_check_texture(lua_State* l, int idx) {
    return lua_check_managed_obj<LuaTexture, TEXTURE_TNAME>(l, idx);
  }
  LuaTexture lua_cast_texture(lua_State* l, int idx) {
    if(lua_isnumber(l, idx)) {
      return const_texture(lua_tonumber(l, idx));
    } else if(lua_test_spectrum(l, idx)) {
      return const_texture(lua_check_spectrum(l, idx));
    }
    return lua_check_texture(l, idx);
  }
  bool lua_test_texture(lua_State* l, int idx) {
    return lua_test_managed_obj<LuaTexture, TEXTURE_TNAME>(l, idx);
  }
  void lua_push_texture(lua_State* l, const LuaTexture& vec) {
    return lua_push_managed_obj<LuaTexture, TEXTURE_TNAME>(l, vec);
  }
}

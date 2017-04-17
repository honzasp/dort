#include "dort/basic_textures.hpp"
#include "dort/color_maps.hpp"
#include "dort/image.hpp"
#include "dort/image_texture.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_spectrum.hpp"
#include "dort/lua_texture.hpp"
#include "dort/lua_texture_magic.hpp"
#include "dort/noise_texture.hpp"
#include "dort/render_texture.hpp"
#include "dort/texture_maps.hpp"

namespace dort {
  int lua_open_texture(lua_State* l) {
    const luaL_Reg texture_methods[] = {
      {"__concat", lua_texture_compose},
      {"__add", lua_texture_add},
      {"__mul", lua_texture_mul},
      {"__gc", lua_gc_managed_obj<LuaTexture, TEXTURE_TNAME>},
      {0, 0},
    };

    const luaL_Reg texture_funs[] = {
      {"compose", lua_texture_compose},
      {"make_const_geom", lua_texture_make_const<const DiffGeom&>},
      {"make_const_1d", lua_texture_make_const<float>},
      {"make_const_2d", lua_texture_make_const<Vec2>},
      {"make_const_3d", lua_texture_make_const<Vec3>},
      {"make_identity_1d", lua_texture_make_identity<float>},
      {"make_identity_2d", lua_texture_make_identity<Vec2>},
      {"make_identity_3d", lua_texture_make_identity<Vec3>},
      {"make_lerp", lua_texture_make_lerp},
      {"make_checkerboard_1d", lua_texture_make_checkerboard<float>},
      {"make_checkerboard_2d", lua_texture_make_checkerboard<Vec2>},
      {"make_checkerboard_3d", lua_texture_make_checkerboard<Vec3>},
      {"make_value_noise_1d", lua_texture_make_value_noise<float, float>},
      {"make_value_noise_2d", lua_texture_make_value_noise<float, Vec2>},
      {"make_value_noise_3d", lua_texture_make_value_noise<float, Vec3>},
      {"make_value_noise_1d_of_2d", lua_texture_make_value_noise<Vec2, float>},
      {"make_value_noise_2d_of_2d", lua_texture_make_value_noise<Vec2, Vec2>},
      {"make_value_noise_3d_of_2d", lua_texture_make_value_noise<Vec2, Vec3>},
      {"make_value_noise_1d_of_3d", lua_texture_make_value_noise<Vec3, float>},
      {"make_value_noise_2d_of_3d", lua_texture_make_value_noise<Vec3, Vec2>},
      {"make_value_noise_3d_of_3d", lua_texture_make_value_noise<Vec3, Vec3>},
      {"make_gain", lua_texture_make_gain},
      {"make_bias", lua_texture_make_bias},
      {"make_average", lua_texture_make_average},
      {"make_image", lua_texture_make_image},
      {"make_map_uv", lua_texture_map_make_uv},
      {"make_map_xy", lua_texture_map_make_xy},
      {"make_map_spherical", lua_texture_map_make_spherical},
      {"make_map_cylindrical", lua_texture_map_make_cylindrical},
      {"make_map_xyz", lua_texture_map_make_xyz},
      {"make_color_map_grayscale", lua_texture_color_map_make_grayscale},
      {"make_color_map_lerp", lua_texture_color_map_make_lerp},
      {"make_color_map_spline", lua_texture_color_map_make_spline},
      {"render_2d", lua_texture_render_2d},
      {0, 0},
    };

    lua_register_type(l, TEXTURE_TNAME, texture_methods);
    luaL_newlib(l, texture_funs);
    return 1;
  }

  template<class Middle>
  struct LuaTextureComposeLambda {
    template<class Out, class In>
    struct L {
      static void handle(LuaTexture& lua_tex, 
          const LuaTexture& lua_tex_1, const LuaTexture& lua_tex_2)
      {
        auto tex_1 = lua_tex_1.get<Out, Middle>();
        auto tex_2 = lua_tex_2.get<Middle, In>();
        lua_tex.texture = compose_texture<Out, Middle, In>(tex_1, tex_2);
      }
    };
  };

  int lua_texture_compose(lua_State* l) {
    LuaTexture lua_tex_1 = lua_check_texture(l, 1);
    LuaTexture lua_tex_2 = lua_check_texture(l, 2);

    LuaTexture lua_tex;
    lua_tex.in_type = lua_tex_2.in_type;
    lua_tex.out_type = lua_tex_1.out_type;

    if(lua_tex_1.in_type == LuaTextureIn::Float &&
        lua_tex_2.out_type == LuaTextureOut::Float) 
    {
      lua_texture_dispatch_out_in<LuaTextureComposeLambda<float>::template L>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_1, lua_tex_2);
    } else if(lua_tex_1.in_type == LuaTextureIn::Vec2 &&
        lua_tex_2.out_type == LuaTextureOut::Vec2) 
    {
      lua_texture_dispatch_out_in<LuaTextureComposeLambda<Vec2>::template L>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_1, lua_tex_2);
    } else if(lua_tex_1.in_type == LuaTextureIn::Vec3 &&
        lua_tex_2.out_type == LuaTextureOut::Vec3) 
    {
      lua_texture_dispatch_out_in<LuaTextureComposeLambda<Vec3>::template L>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_1, lua_tex_2);
    } else if(lua_tex_1.in_type == LuaTextureIn::Spectrum &&
        lua_tex_2.out_type == LuaTextureOut::Spectrum) 
    {
      lua_texture_dispatch_out_in<LuaTextureComposeLambda<Spectrum>::template L>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_1, lua_tex_2);
    } else {
      return luaL_error(l, "The input type of the first texture "
          "must match the output type of the second texture");
    }

    lua_push_texture(l, lua_tex);
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

  template<class OutIn> int lua_texture_make_identity(lua_State* l) {
    LuaTexture lua_tex;
    lua_tex.in_type = lua_texture_in_v<OutIn>();
    lua_tex.out_type = lua_texture_out_v<OutIn>();
    lua_tex.texture = identity_texture<OutIn>();
    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureLerpLambda {
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

    lua_texture_dispatch_out_in<LuaTextureLerpLambda>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_t, lua_tex_0, lua_tex_1);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class In>
  struct LuaTextureCheckerboardLambda {
    template<class Out>
    struct L {
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

    lua_texture_dispatch_out<LuaTextureCheckerboardLambda<In>::template L>(
        lua_tex.out_type,
        lua_tex, even_lua_tex, odd_lua_tex, check_size);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In> int lua_texture_make_value_noise(lua_State* l) {
    std::vector<ValueNoiseLayer> layers; {
      int32_t count = lua_rawlen(l, 1);
      for(int32_t i = 1; i <= count; ++i) {
        ValueNoiseLayer layer;
        lua_rawgeti(l, 1, i);

        lua_getfield(l, -1, "frequency");
        layer.frequency = luaL_checknumber(l, -1);
        lua_pop(l, 1);

        lua_getfield(l, -1, "weight");
        layer.weight = luaL_checknumber(l, -1);
        lua_pop(l, 1);

        layers.push_back(layer);
        lua_pop(l, 1);
      }
    }

    LuaTexture lua_tex;
    lua_tex.in_type = lua_texture_in_v<In>();
    lua_tex.out_type = lua_texture_out_v<Out>();
    lua_tex.texture = value_noise_texture<Out, In>(std::move(layers));
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_make_image(lua_State* l) {
    auto image = lua_check_image_8(l, 1);

    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Vec2;
    lua_tex.out_type = LuaTextureOut::Spectrum;
    lua_tex.texture = std::make_shared<ImageTexture>(image);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureAddLambda {
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

    lua_texture_dispatch_out_in<LuaTextureAddLambda>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_0, lua_tex_1);

    lua_push_texture(l, lua_tex);
    return 1;
  }

  template<class Out, class In>
  struct LuaTextureMulLambda {
    static void handle(LuaTexture& lua_tex,
        const LuaTexture& lua_tex_0, const LuaTexture& lua_tex_1)
    {
      auto tex_0 = lua_tex_0.get<Out, In>();
      auto tex_1 = lua_tex_1.get<Out, In>();
      lua_tex.texture = mul_texture(tex_0, tex_1);
    }
  };

  template<class Out, class In>
  struct LuaTextureScaleLambda {
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
      return luaL_error(l, "Textures that are multiplied must have the same input type");
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
      lua_texture_dispatch_out_in<LuaTextureMulLambda>(
          lua_tex.out_type, lua_tex.in_type,
          lua_tex, lua_tex_0, lua_tex_1);
    } else {
      return luaL_error(l, "Incompatible textures to multiply");
    }

    LuaTexture lua_tex;
    lua_tex.out_type = lua_tex_other.out_type;
    lua_tex.in_type = lua_tex_other.in_type;
    lua_texture_dispatch_out_in<LuaTextureScaleLambda>(
        lua_tex.out_type, lua_tex.in_type,
        lua_tex, lua_tex_float, lua_tex_other);

    lua_push_texture(l, lua_tex);
    return 1;
  }


  int lua_texture_make_gain(lua_State* l) {
    float g = luaL_checknumber(l, 1);
    lua_push_texture(l, gain_texture(g));
    return 1;
  }

  int lua_texture_make_bias(lua_State* l) {
    float b = luaL_checknumber(l, 1);
    lua_push_texture(l, bias_texture(b));
    return 1;
  }

  int lua_texture_make_average(lua_State* l) {
    lua_push_texture(l, average_texture());
    return 1;
  }

  int lua_texture_map_make_uv(lua_State* l) {
    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Geom;
    lua_tex.out_type = LuaTextureOut::Vec2;
    lua_tex.texture = uv_texture_map();
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_map_make_xy(lua_State* l) {
    auto trans = lua_check_transform(l, 1);
    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Geom;
    lua_tex.out_type = LuaTextureOut::Vec2;
    lua_tex.texture = xy_texture_map(trans);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_map_make_cylindrical(lua_State* l) {
    auto trans = lua_check_transform(l, 1);
    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Geom;
    lua_tex.out_type = LuaTextureOut::Vec2;
    lua_tex.texture = cylindrical_texture_map(trans);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_map_make_spherical(lua_State* l) {
    auto trans = lua_check_transform(l, 1);
    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Geom;
    lua_tex.out_type = LuaTextureOut::Vec2;
    lua_tex.texture = spherical_texture_map(trans);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_map_make_xyz(lua_State* l) {
    auto trans = lua_check_transform(l, 1);
    LuaTexture lua_tex;
    lua_tex.in_type = LuaTextureIn::Geom;
    lua_tex.out_type = LuaTextureOut::Vec3;
    lua_tex.texture = xyz_texture_map(trans);
    lua_push_texture(l, lua_tex);
    return 1;
  }

  int lua_texture_color_map_make_grayscale(lua_State* l) {
    lua_push_texture(l, grayscale_color_map());
    return 1;
  }

  int lua_texture_color_map_make_lerp(lua_State* l) {
    Spectrum color_0 = lua_check_spectrum(l, 1);
    Spectrum color_1 = lua_check_spectrum(l, 2);
    lua_push_texture(l, lerp_color_map(color_0, color_1));
    return 1;
  }

  int lua_texture_color_map_make_spline(lua_State* l) {
    std::vector<Spectrum> knots; {
      int32_t len = lua_rawlen(l, 1);
      for(int32_t i = 1; i <= len; ++i) {
        lua_rawgeti(l, 1, i);
        knots.push_back(lua_check_spectrum(l, -1));
        lua_pop(l, 1);
      }
    }

    if(knots.size() < 4) {
      return luaL_argerror(l, 1, "At least four knots must be given "
          "(the first and last knots are not part of the spline, "
          "but define the derivative at the end points)");
    }

    lua_push_texture(l, spline_color_map(knots));
    return 1;
  }

  int lua_texture_render_2d(lua_State* l) {
    LuaTexture lua_tex = lua_check_texture(l, 1);

    int p = 2;
    uint32_t res = lua_param_uint32_opt(l, p, "res", 400);
    uint32_t x_res = lua_param_uint32_opt(l, p, "x_res", res);
    uint32_t y_res = lua_param_uint32_opt(l, p, "y_res", res);
    float scale = lua_param_float_opt(l, p, "scale", 1.f);
    float x_scale = lua_param_float_opt(l, p, "scale", 
        scale * float(y_res) / float(max(x_res, y_res)));
    float y_scale = lua_param_float_opt(l, p, "scale", 
        scale * float(x_res) / float(max(x_res, y_res)));
    lua_params_check_unused(l, p);

    if(lua_tex.in_type != LuaTextureIn::Vec2) {
      return luaL_argerror(l, 1, "Only textures with Vec2 input can be rendered");
    }

    std::shared_ptr<Texture2d<Spectrum>> tex;
    if(lua_tex.out_type == LuaTextureOut::Float) {
      tex = compose_texture(grayscale_color_map(), lua_tex.get<float, Vec2>());
    } else if(lua_tex.out_type == LuaTextureOut::Spectrum) {
      tex = lua_tex.get<Spectrum, Vec2>();
    } else {
      return luaL_argerror(l, 1,
          "Only textures with float or spectrum output can be rendered");
    }

    auto image = std::make_shared<Image<PixelRgb8>>(render_texture_2d(tex,
          Vec2i(x_res, y_res), Vec2(float(x_res) * x_scale, float(y_res) * y_scale)));
    lua_push_image_8(l, image);
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

/// Textures.
// A texture in its general form is simply a function `In -> Out`, where the
// supported types for `In` are
//
// - `float`
// - `Vec2`
// - `Vec3`
// - `Spectrum`
// - `DiffGeom` -- an internal type that describes the local -- differential --
// geometry at a point in scene
//
// and for `Out` these are:
//
// - `float`
// - `Vec2`
// - `Vec3`
// - `Spectrum`
//
// This module provides a range of functions that define primitive
// textures or compose multiple textures to form new textures.
//
// The textures are used when defining materials (see @{dort.material}) -- in
// this case the input type is always `DiffGeom`. To get a `Vec2` or `Vec3` from
// the `DiffGeom`, one of the "Geometry mapping" textures may be used.
//
// The textures also have a few methods that support convenient binary
// operators:
//
// - `__concat` (the `..` operator) -- @{compose}
// - `__add` (the `+` operator) -- @{add}
// - `__mul` (the `*` operator) -- @{multiply}
//
// @module dort.texture
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
      /// Combinators.
      // @section

      /// Functional composition (`B -> C` and `A -> B`).
      // If the type of `tex_1` is `B -> C` and the type of `tex_2` is `A -> B`,
      // the resulting texture has type `A -> C`.
      // @function compose
      // @param tex_1
      // @param tex_2
      {"compose", lua_texture_compose},

      /// Add textures (`A -> B` and `A -> B`).
      // The textures must have the same input and output types.
      // @function add
      // @param tex_1
      // @param tex_2
      {"add", lua_texture_add},

      /// Multiply textures (`A -> B` and `A -> C`).
      // The textures must have the same input type, and the output types must
      // either match or at least one of `tex_1`, `tex_2` must output `float`.
      // @function multiply
      // @param tex_1
      // @param tex_2
      {"multiply", lua_texture_mul},

      /// Linear interpolation between textures.
      //
      // - `t` -- `A -> float`, determines the ratio between `tex_0` and `tex_1`
      // (where 0 is only `tex_0` and 1 is only `tex_1`).
      // - `tex_0`, `tex_1` -- `A -> B`, are the composed textures.
      //
      // The resulting texture has type `A -> B`.
      //
      // @function lerp
      // @param params
      {"lerp", lua_texture_make_lerp},

      /// Checkerboard
      // @section

      /// Checkerboard pattern `A -> B` (for `A = float`).
      // Makes a texture that alternates between `even` and `odd` in a
      // checkerboard pattern. The input type `A` can be `float`, `Vec2` or
      // `Vec3`.
      //
      // - `check_size` -- the size of one check (1 by default)
      // - `even`, `odd` -- `A -> B`, the two textures that alternate
      //
      // @function checkerboard_1d
      // @param params
      {"checkerboard_1d", lua_texture_make_checkerboard},
      /// Checkerboard pattern `A -> B` (for `A = Vec2`).
      // @function checkerboard_2d
      // @param params
      {"checkerboard_2d", lua_texture_make_checkerboard},
      /// Checkerboard pattern `A -> B` (for `A = Vec3`).
      // @function checkerboard_3d
      // @param params
      {"checkerboard_3d", lua_texture_make_checkerboard},

      /// Noise
      // @section

      /// Value noise `A -> B` (for `A = float, B = float`).
      // Makes a texture from multiple layers of "value noise". Each layer
      // corresponds to an integral lattice of points with random values `B`,
      // and the value at a particular point `A` is formed by interpolating the
      // random values at nearby points of the lattice.
      //
      // `layers` defines any number of layers, each layer is defined by a
      // table:
      //
      // - `frequency` -- the number of lattice points per unit of `A`
      // - `weight` -- the relative weight of this layer w.r.t. other layers
      //
      // @function value_noise_1d
      // @param layers
      {"value_noise_1d", lua_texture_make_value_noise<float, float>},
      /// Value noise `A -> B` (for `A = Vec2, B = float`).
      // @function value_noise_2d
      // @param layers
      {"value_noise_2d", lua_texture_make_value_noise<float, Vec2>},
      /// Value noise `A -> B` (for `A = Vec3, B = float`).
      // @function value_noise_3d
      // @param layers
      {"value_noise_3d", lua_texture_make_value_noise<float, Vec3>},
      /// Value noise `A -> B` (for `A = float, B = Vec2`).
      // @function value_noise_1d_of_2d
      // @param layers
      {"value_noise_1d_of_2d", lua_texture_make_value_noise<Vec2, float>},
      /// Value noise `A -> B` (for `A = Vec2, B = Vec2`).
      // @function value_noise_2d_of_2d
      // @param layers
      {"value_noise_2d_of_2d", lua_texture_make_value_noise<Vec2, Vec2>},
      /// Value noise `A -> B` (for `A = Vec3, B = Vec2`).
      // @function value_noise_3d_of_2d
      // @param layers
      {"value_noise_3d_of_2d", lua_texture_make_value_noise<Vec2, Vec3>},
      /// Value noise `A -> B` (for `A = float, B = Vec3`).
      // @function value_noise_1d_of_3d
      // @param layers
      {"value_noise_1d_of_3d", lua_texture_make_value_noise<Vec3, float>},
      /// Value noise `A -> B` (for `A = Vec2, B = Vec3`).
      // @function value_noise_2d_of_3d
      // @param layers
      {"value_noise_2d_of_3d", lua_texture_make_value_noise<Vec3, Vec2>},
      /// Value noise `A -> B` (for `A = Vec3, B = Vec3`).
      // @function value_noise_3d_of_3d
      // @param layers
      {"value_noise_3d_of_3d", lua_texture_make_value_noise<Vec3, Vec3>},

      /// Simple functions
      // @section simple

      /// Gain function `float -> float`.
      // The _gain(x)_ function is defined as _gain(x) = bias(1-g, 2*x) * 0.5_
      // if _x < 0.5_, _gain(x) = 1 - bias(1-g, 2 - 2*x) * 0.5_ if _x > 0.5_,
      // where _g_ is a parameter.
      // @function gain
      // @param g
      {"gain", lua_texture_make_gain},

      /// Bias function `float -> float`.
      // The _bias(x)_ function is defined as _bias(x) = x / ((1/b - 2) * (1 -
      // x) + 1)_, where _b_ is a parameter.
      // @function bias
      // @param b
      {"bias", lua_texture_make_bias},

      /// Spectrum average `Spectrum -> float`.
      // Maps spectra to the average value of the channels. (Currently, this is
      // just arithmetic mean of the red, green and blue channels).
      // @function average
      {"average", lua_texture_make_average},

      /// Image texture
      // @section image

      /// Image texture `Vec2 -> Spectrum`.
      // The texture maps the image into the rectangle `(0,0),(1,1)`, with the
      // image repeating in both axes. `img` is a LDR image (`Image.Rgb8`).
      // @function image
      // @param img
      {"image", lua_texture_make_image},

      /// Geometry mapping.
      // @section map

      /// UV texture mapping (`DiffGeom -> Vec2`).
      // Gets the UV coordinates from the input geometry.
      // @function map_uv
      {"map_uv", lua_texture_map_make_uv},

      /// XY texture mapping (`DiffGeom -> Vec2`).
      // Applies the inverse of the texture-to-world transform `transform` to
      // the world coordinates of the input geometry and returns the XY
      // coordinates.
      // @function map_xy
      // @param transform
      {"map_xy", lua_texture_map_make_xy},

      /// Spherical texture mapping (`DiffGeom -> Vec2`).
      // Applies the inverse of the texture-to-world transform `transform` to
      // the world coordinates of the input geometry and returns the _(phi,
      // theta)_ spherical coordinates.
      // @function map_spherical
      // @param transform
      {"map_spherical", lua_texture_map_make_spherical},

      /// Spherical texture mapping (`DiffGeom -> Vec2`).
      // Applies the inverse of the texture-to-world transform `transform` to
      // the world coordinates of the input geometry and returns the _(phi, z)_
      // cylindrical coordinates.
      // @function map_cylindrical
      // @param transform
      {"map_cylindrical", lua_texture_map_make_cylindrical},
      //
      /// XYZ texture mapping (`DiffGeom -> Vec2`).
      // Applies the inverse of the texture-to-world transform `transform` to
      // the world coordinates of the input geometry and returns the XYZ
      // coordinates.
      // @function map_xyz
      // @param transform
      {"map_xyz", lua_texture_map_make_xyz},

      /// Color mapping
      // @section color_map

      /// Grayscale color map (`float -> Spectrum`).
      // Maps input _x_ into spectrum _x_.
      // @function color_map_grayscale
      {"color_map_grayscale", lua_texture_color_map_make_grayscale},

      /// Linear interpolation between spectra (`float -> Spectrum`).
      // Maps input _x_ to _lerp(x, color_0, color_1)_.
      // @function color_map_lerp
      // @param color_0
      // @param color_1
      {"color_map_lerp", lua_texture_color_map_make_lerp},

      /// Spline color map (`float -> Spectrum`).
      // Defines a spline that maps values between 0 and 1 to color values. The
      // spline is defined by `knots`, which define the colors at regularly
      // separated points in the range `[0, 1]`. The point 0 is defined by the
      // second knot, the first knot affects only the derivative here
      // (similarly, the point 1 is defined by the second to last knot); hence,
      // at least 4 knots must be defined.
      // @function color_map_spline
      // @param knots
      {"color_map_spline", lua_texture_color_map_make_spline},

      /// Rendering
      // @section render

      /// Render a texture into image.
      // Renders the `texture` (of type `Vec2 -> Spectrum` or `Vec2 -> float`)
      // into `Image.Rgb8`. The `params` are:
      //
      // - `res` (or `x_res` and `y_res`) -- the resolution of the output image
      // (400 pixels by default)
      // - `scale` (or `x_scale` and `y_scale`) -- the size of the rendered
      // rectangle in the texture space (1 by default)
      //
      // @function render_2d
      // @param texture
      // @param params
      {"render_2d", lua_texture_render_2d},

      /// Helpers
      // @section helpers

      /// Make constant `float -> A`.
      // The function always has value `x`, the output type `A` is inferred from
      // the type of `x`.
      // @function const_1d
      // @param x
      {"const_1d", lua_texture_make_const<float>},
      /// Make constant `Vec2 -> A`.
      // @function const_2d
      // @param x
      {"const_2d", lua_texture_make_const<Vec2>},
      /// Make constant `Vec3 -> A`.
      // @function const_2d
      // @param x
      {"const_3d", lua_texture_make_const<Vec3>},
      /// Make constant `DiffGeom -> A`.
      // @function const_geom
      // @param x
      {"const_geom", lua_texture_make_const<const DiffGeom&>},

      /// Make identity `float -> float`.
      // @function identity_1d
      {"identity_1d", lua_texture_make_identity<float>},
      /// Make identity `Vec2 -> Vec2`.
      // @function identity_2d
      {"identity_2d", lua_texture_make_identity<Vec2>},
      /// Make identity `Vec3 -> Vec3`.
      // @function identity_3d
      {"identity_3d", lua_texture_make_identity<Vec3>},

      /// @section end

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
    } else if(lua_test_vec2(l, 1)) {
      lua_tex.out_type = LuaTextureOut::Vec2;
      lua_tex.texture = const_texture<Vec2, In>(lua_check_vec2(l, 1));
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
        auto even_tex = even_lua_tex.get<Out, In>();
        auto odd_tex = odd_lua_tex.get<Out, In>();
        lua_tex.texture = checkerboard_texture<Out, In>(
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
    float x_scale = lua_param_float_opt(l, p, "x_scale", 
        scale * float(y_res) / float(max(x_res, y_res)));
    float y_scale = lua_param_float_opt(l, p, "y_scale", 
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

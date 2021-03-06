/// Materials.
// Most properties of materials can be textured (see @{dort.texture}), so that
// the appearance of a material can vary. However, every time a function expects
// a texture, a single value of the required type can be passed -- it will be
// silently converted into a constant texture.
//
// The output types of the textures vary, but the input to all material
// textures is always `DiffGeom`.
// @module dort.material
#include "dort/basic_textures.hpp"
#include "dort/bump_material.hpp"
#include "dort/dielectric_material.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_texture_magic.hpp"
#include "dort/lambert_material.hpp"
#include "dort/mirror_material.hpp"
#include "dort/oren_nayar_material.hpp"
#include "dort/phong_material.hpp"
#include "dort/rough_dielectric_material.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  int lua_open_material(lua_State* l) {
    const luaL_Reg material_methods[] = {
      {"__gc", lua_gc_shared_obj<Material, MATERIAL_TNAME>},
      {0, 0},
    };

    const luaL_Reg material_funs[] = {
      {"make_lambert", lua_material_make_lambert},
      {"make_oren_nayar", lua_material_make_oren_nayar},
      {"make_mirror", lua_material_make_mirror},
      {"make_dielectric", lua_material_make_dielectric},
      {"make_rough_dielectric", lua_material_make_rough_dielectric},
      {"make_phong", lua_material_make_phong},
      {"make_bump", lua_material_make_bump},
      {0, 0},
    };

    lua_register_type(l, MATERIAL_TNAME, material_methods);
    luaL_newlib(l, material_funs);
    return 1;
  }

  /// Make a lambertian material.
  //
  // - `albedo` -- albedo of the BRDF (1 by default) (`Spectrum` texture)
  //
  // @function make_lambert
  // @param params
  int lua_material_make_lambert(lua_State* l) {
    int p = 1;
    auto albedo = lua_param_texture_opt(l, p, "albedo", 
        const_texture<Spectrum>(Spectrum(1.f))).check<Spectrum>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<LambertMaterial>(albedo));
    return 1;
  }

  /// Make an Oren-Nayar material.
  //
  // - `albedo` -- albedo of the BRDF (1 by default) (`Spectrum` texture)
  // - `sigma` -- the roughness parameter (0.1 by default) (`float` texture)
  //
  // @function make_oren_nayar
  // @param params
  int lua_material_make_oren_nayar(lua_State* l) {
    int p = 1;
    auto albedo = lua_param_texture_opt(l, p, "albedo", 
        const_texture<Spectrum>(Spectrum(1.f))).check<Spectrum>(l);
    auto sigma = lua_param_texture_opt(l, p, "sigma",
        const_texture<float>(0.1f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<OrenNayarMaterial>(albedo, sigma));
    return 1;
  }

  /// Make an ideal mirror material.
  //
  // - `albedo` -- albedo of the BRDF (1 by default) (`Spectrum` texture)
  //
  // @function make_mirror
  // @param params
  int lua_material_make_mirror(lua_State* l) {
    int p = 1;
    auto albedo = lua_param_texture_opt(l, p, "albedo",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    lua_params_check_unused(l, p);

    lua_push_material(l, std::make_shared<MirrorMaterial>(albedo));
    return 1;
  }

  /// Make an ideal dielectric material.
  //
  // - `reflect_tint` -- non-physical multiplier for the reflected radiance (1
  // by default) (`Spectrum` texture)
  // - `transmit_tint` -- non-physical multiplier for the transmitted radiance
  // (1 by default) (`Spectrum` texture)
  // - `ior_inside` -- index of refraction inside the body (1.5 by default)
  // - `ior_outside` -- index of refraction outside of the body (1.5 by default)
  // - `is_thin` -- if true, the surface is assumed to be infinitely thin slab
  // of dielectric with IOR `ior_inside` surrounded on both sides with
  // `ior_outside`. The material cheaply accounts for multiple reflections
  // inside the slab.
  //
  // @function make_dielectric
  // @param params
  int lua_material_make_dielectric(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "reflect_tint",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto transmit = lua_param_texture_opt(l, p, "transmit_tint",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    float ior_inside = lua_param_float_opt(l, p, "ior_inside", 1.5f);
    float ior_outside = lua_param_float_opt(l, p, "ior_outside", 1.f);
    bool is_thin = lua_param_bool_opt(l, p, "is_thin", false);
    lua_params_check_unused(l, p);

    lua_push_material(l, std::make_shared<DielectricMaterial>(
          reflect, transmit, ior_inside, ior_outside, is_thin));
    return 1;
  }

  /// Make a rough microfacet dielectric material.
  //
  // - `reflect_tint` -- non-physical multiplier for the reflected radiance (1
  // by default) (`Spectrum` texture)
  // - `transmit_tint` -- non-physical multiplier for the transmitted radiance
  // (1 by default) (`Spectrum` texture)
  // - `ior_inside` -- index of refraction inside the body (1.5 by default)
  // - `ior_outside` -- index of refraction outside of the body (1.5 by default)
  // - `distribution` -- the microfacet distribution, `beckmann` (default),
  // `phong` or `ggx`.
  // - `roughness` -- the roughness parameter for the microfacet distribution.
  // (`float` texture)
  int lua_material_make_rough_dielectric(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "reflect_tint",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto transmit = lua_param_texture_opt(l, p, "transmit_tint",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto roughness = lua_param_texture_opt(l, p, "roughness",
        const_texture(0.1f)).check<float>(l);
    float ior_inside = lua_param_float_opt(l, p, "ior_inside", 1.5f);
    float ior_outside = lua_param_float_opt(l, p, "ior_outside", 1.f);
    auto distrib_str = lua_param_string_opt(l, p, "distribution", "beckmann");
    lua_params_check_unused(l, p);

    auto distrib = lua_get_microfacet_type(l, distrib_str);

    lua_push_material(l, std::make_shared<RoughDielectricMaterial>(
          reflect, transmit, roughness, ior_inside, ior_outside, distrib));
    return 1;
  }

  /// Make a Phong material.
  //
  // - `albedo` -- the diffuse albedo of the BRDF (`Spectrum` texture)
  // - `glossy_albedo` -- the glossy albedo of the BRDF (`Spectrum` texture)
  // - `exponent` -- exponent of the Phong distribution (`float` texture)
  //
  // @function make_phong
  // @param params
  int lua_material_make_phong(lua_State* l) {
    int p = 1;
    auto diffuse = lua_param_texture_opt(l, p, "albedo",
        const_texture(Spectrum(0.f))).check<Spectrum>(l);
    auto glossy = lua_param_texture_opt(l, p, "glossy_albedo",
        diffuse).check<Spectrum>(l);
    auto exponent = lua_param_texture_opt(l, p, "exponent",
        const_texture(50.f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<PhongMaterial>(
          diffuse, glossy, exponent));
    return 1;
  }

  /// Make a bump-mapped material.
  //
  // - `bump` -- the displacement (`float` texture)
  // - `material` -- the substrate material
  //
  // @function make_bump
  // @param params
  int lua_material_make_bump(lua_State* l) {
    int p = 1;
    auto displac = lua_param_texture(l, p, "bump").check<float>(l);
    auto material = lua_param_material(l, p, "material");
    lua_params_check_unused(l, p);

    lua_push_material(l, std::make_shared<BumpMaterial>(
          displac, material));
    return 1;
  }

  MicrofacetType lua_get_microfacet_type(lua_State* l, const std::string& type) {
    if(type == "beckmann") {
      return MicrofacetType::Beckmann;
    } else if(type == "phong") {
      return MicrofacetType::Phong;
    } else if(type == "ggx") {
      return MicrofacetType::GGX;
    } else {
      luaL_error(l, "Unknwon microfacet type '%s'", type.c_str());
      return MicrofacetType::Beckmann;
    }
  }

  std::shared_ptr<Material> lua_check_material(lua_State* l, int idx) {
    return lua_check_shared_obj<Material, MATERIAL_TNAME>(l, idx);
  }
  bool lua_test_material(lua_State* l, int idx) {
    return lua_test_shared_obj<Material, MATERIAL_TNAME>(l, idx);
  }
  void lua_push_material(lua_State* l, std::shared_ptr<Material> material) {
    lua_push_shared_obj<Material, MATERIAL_TNAME>(l, material);
  }
}

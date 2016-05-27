#include "dort/basic_textures.hpp"
#include "dort/bump_material.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_texture_magic.hpp"
#include "dort/matte_material.hpp"
#include "dort/metal_material.hpp"
#include "dort/phong_material.hpp"
#include "dort/plastic_material.hpp"
#include "dort/rough_glass_material.hpp"
#include "dort/spectrum.hpp"
#include "dort/specular_materials.hpp"

namespace dort {
  int lua_open_material(lua_State* l) {
    const luaL_Reg material_methods[] = {
      {"__gc", lua_gc_shared_obj<Material, MATERIAL_TNAME>},
      {0, 0},
    };

    const luaL_Reg material_funs[] = {
      {"make_matte", lua_material_make_matte},
      {"make_plastic", lua_material_make_plastic},
      {"make_metal", lua_material_make_metal},
      {"make_mirror", lua_material_make_mirror},
      {"make_glass", lua_material_make_glass},
      {"make_rough_glass", lua_material_make_rough_glass},
      {"make_phong", lua_material_make_phong},
      {"make_bump", lua_material_make_bump},
      {0, 0},
    };

    lua_register_type(l, MATERIAL_TNAME, material_methods);
    luaL_newlib(l, material_funs);
    return 1;
  }

  int lua_material_make_matte(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "color", 
        const_texture<Spectrum>(Spectrum(1.f))).check<Spectrum>(l);
    auto sigma = lua_param_texture_opt(l, p, "sigma",
        const_texture<float>(0.f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<MatteMaterial>(reflect, sigma));
    return 1;
  }

  int lua_material_make_plastic(lua_State* l) {
    int p = 1;
    auto diffuse = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto reflect = lua_param_texture_opt(l, p, "reflect_color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto roughness = lua_param_texture_opt(l, p, "roughness",
        const_texture(0.2f)).check<float>(l);
    auto eta = lua_param_texture_opt(l, p, "eta",
        const_texture(1.5f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<PlasticMaterial>(
          diffuse, reflect, roughness, eta));
    return 1;
  }

  int lua_material_make_metal(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto roughness = lua_param_texture_opt(l, p, "roughness",
        const_texture(0.2f)).check<float>(l);
    auto eta = lua_param_texture_opt(l, p, "eta",
        const_texture(1.f)).check<float>(l);
    auto k = lua_param_texture_opt(l, p, "k",
        const_texture(5.f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<MetalMaterial>(
          reflect, roughness, eta, k));
    return 1;
  }

  int lua_material_make_mirror(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto eta = lua_param_texture_opt(l, p, "eta",
        const_texture(1.5f)).check<float>(l);
    lua_params_check_unused(l, p);

    lua_push_material(l, std::make_shared<MirrorMaterial>(reflect, eta));
    return 1;
  }

  int lua_material_make_glass(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto transmit = lua_param_texture_opt(l, p, "transmit_color",
        reflect).check<Spectrum>(l);
    auto eta = lua_param_texture_opt(l, p, "eta",
        const_texture(1.5f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<GlassMaterial>(reflect, transmit, eta));
    return 1;
  }

  int lua_material_make_rough_glass(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto transmit = lua_param_texture_opt(l, p, "transmit_color",
        reflect).check<Spectrum>(l);
    auto roughness = lua_param_texture_opt(l, p, "roughness",
        const_texture(0.2f)).check<float>(l);
    auto eta = lua_param_texture_opt(l, p, "eta",
        const_texture(1.5f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<RoughGlassMaterial>(
          reflect, transmit, roughness, eta));
    return 1;
  }

  int lua_material_make_phong(lua_State* l) {
    int p = 1;
    auto diffuse = lua_param_texture_opt(l, p, "color",
        const_texture(Spectrum(0.f))).check<Spectrum>(l);
    auto glossy = lua_param_texture_opt(l, p, "glossy_color",
        const_texture(Spectrum(1.f))).check<Spectrum>(l);
    auto exponent = lua_param_texture_opt(l, p, "exponent",
        const_texture(50.f)).check<float>(l);
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<PhongMaterial>(
          diffuse, glossy, exponent));
    return 1;
  }

  int lua_material_make_bump(lua_State* l) {
    int p = 1;
    auto displac = lua_param_texture(l, p, "bump").check<float>(l);
    auto material = lua_param_material(l, p, "material");
    lua_params_check_unused(l, p);

    lua_push_material(l, std::make_shared<BumpMaterial>(
          displac, material));
    return 1;
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

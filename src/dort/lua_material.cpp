#include "dort/basic_textures.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_params.hpp"
#include "dort/matte_material.hpp"
#include "dort/plastic_material.hpp"
#include "dort/spectrum.hpp"
#include "dort/specular_materials.hpp"

namespace dort {
  int lua_open_material(lua_State* l) {
    const luaL_Reg material_methods[] = {
      {"__gc", lua_gc_shared_obj<Material, MATERIAL_TNAME>},
      {0, 0},
    };
    lua_register_type(l, MATERIAL_TNAME, material_methods);
    lua_register(l, "matte_material", lua_material_make_matte);
    lua_register(l, "plastic_material", lua_material_make_plastic);
    lua_register(l, "mirror_material", lua_material_make_mirror);
    lua_register(l, "glass_material", lua_material_make_glass);
    return 0;
  }

  int lua_material_make_matte(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_spectrum_opt(l, p, "color", 
        const_texture(Spectrum(1.f)));
    auto sigma = lua_param_texture_float_opt(l, p, "sigma",
        const_texture(0.f));
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<MatteMaterial>(reflect, sigma));
    return 1;
  }

  int lua_material_make_plastic(lua_State* l) {
    int p = 1;
    auto diffuse = lua_param_texture_spectrum_opt(l, p, "color",
        const_texture(Spectrum(1.f)));
    auto reflect = lua_param_texture_spectrum_opt(l, p, "reflect_color",
        const_texture(Spectrum(1.f)));
    auto roughness = lua_param_texture_float_opt(l, p, "roughness", const_texture(1.f));
    auto eta = lua_param_texture_float_opt(l, p, "eta", const_texture(1.5f));
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<PlasticMaterial>(
          diffuse, reflect, roughness, eta));
    return 1;
  }

  int lua_material_make_mirror(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_spectrum_opt(l, p, "color",
        const_texture(Spectrum(1.f)));
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<MirrorMaterial>(reflect));
    return 1;
  }

  int lua_material_make_glass(lua_State* l) {
    int p = 1;
    auto reflect = lua_param_texture_spectrum_opt(l, p, "color",
        const_texture(Spectrum(1.f)));
    auto transmit = lua_param_texture_spectrum_opt(l, p, "transmit_color", reflect);
    auto eta = lua_param_texture_float_opt(l, p, "eta", const_texture(1.5f));
    lua_params_check_unused(l, p);
    lua_push_material(l, std::make_shared<GlassMaterial>(reflect, transmit, eta));
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

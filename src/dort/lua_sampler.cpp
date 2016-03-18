#include "dort/lua_helpers.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_sampler.hpp"
#include "dort/random_sampler.hpp"
#include "dort/stratified_sampler.hpp"

namespace dort {
  int lua_open_sampler(lua_State* l) {
    const luaL_Reg sampler_methods[] = {
      {"__gc", lua_gc_shared_obj<Sampler, SAMPLER_TNAME>},
      {0, 0},
    };

    lua_register_type(l, SAMPLER_TNAME, sampler_methods);
    lua_register(l, "random_sampler", lua_sampler_make_random);
    lua_register(l, "stratified_sampler", lua_sampler_make_stratified);
    return 0;
  }

  int lua_sampler_make_random(lua_State* l) {
    int p = 1;
    uint32_t samples_per_pixel = lua_param_uint32_opt(l, p, "samples_per_pixel", 1);
    uint32_t seed = lua_param_uint32_opt(l, p, "seed", 1);
    lua_params_check_unused(l, p);
    
    lua_push_sampler(l, std::make_shared<RandomSampler>(
          samples_per_pixel, Rng(seed)));
    return 1;
  }

  int lua_sampler_make_stratified(lua_State* l) {
    int p = 1;

    uint32_t samples_per_x = lua_param_uint32_opt(l, p, "samples_per_x", 1);
    uint32_t samples_per_y = lua_param_uint32_opt(l, p, "samples_per_y", 1);
    uint32_t seed = lua_param_uint32_opt(l, p, "seed", 1);
    lua_params_check_unused(l, p);

    lua_push_sampler(l, std::make_shared<StratifiedSampler>(
          samples_per_x, samples_per_y, Rng(seed)));
    return 1;
  }


  std::shared_ptr<Sampler> lua_check_sampler(lua_State* l, int idx) {
    return lua_check_shared_obj<Sampler, SAMPLER_TNAME>(l, idx);
  }
  bool lua_test_sampler(lua_State* l, int idx) {
    return lua_test_shared_obj<Sampler, SAMPLER_TNAME>(l, idx);
  }
  void lua_push_sampler(lua_State* l, std::shared_ptr<Sampler> sampler) {
    lua_push_shared_obj<Sampler, SAMPLER_TNAME>(l, sampler);
  }
}

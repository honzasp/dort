#include <thread>
#include "dort/ctx.hpp"
#include "dort/geometry.hpp"
#include "dort/lua.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_math.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_stats.hpp"
#include "dort/lua_texture.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  int main(int argc, char** argv) {
    stat_init_global();

    uint32_t num_threads = std::thread::hardware_concurrency();
    auto pool_g = std::make_shared<ThreadPool>(num_threads == 0 ? 1 : num_threads);
    CtxG ctx_g(pool_g);

    int exit_status = 3;
    lua_State* l = luaL_newstate();
    lua_set_ctx(l, &ctx_g);
    
    try {
      luaL_requiref(l, "base", luaopen_base, true);
      luaL_requiref(l, MATH_LIBNAME, lua_open_math, true);
      luaL_requiref(l, GEOMETRY_LIBNAME, lua_open_geometry, true);
      luaL_requiref(l, STATS_LIBNAME, lua_open_stats, true);
      luaL_requiref(l, IMAGE_LIBNAME, lua_open_image, true);
      luaL_requiref(l, TEXTURE_LIBNAME, lua_open_texture, true);
      luaL_requiref(l, SHAPE_LIBNAME, lua_open_shape, true);
      luaL_requiref(l, MATERIAL_LIBNAME, lua_open_material, true);
      luaL_requiref(l, LIGHT_LIBNAME, lua_open_light, true);
      luaL_requiref(l, CAMERA_LIBNAME, lua_open_camera, true);
      luaL_requiref(l, BUILDER_LIBNAME, lua_open_builder, true);

      const char* input_file = argc == 1 ? 0 : argv[1];

      int error;
      if((error = luaL_loadfile(l, input_file))) {
        std::fprintf(stderr, "Load error: %s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
        exit_status = 1;
      } else if((error = lua_pcall(l, 0, 0, 0))) {
        std::fprintf(stderr, "Runtime error: %s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
        exit_status = 1;
      } else {
        ctx_g.pool->stop();
        stat_report_global(stderr);
        exit_status = 0;
      }
    } catch(std::exception& exn) {
      std::fprintf(stderr, "Uncaught std::exception: %s\n", exn.what());
      exit_status = 2;
    } catch(...) {
      std::fprintf(stderr, "Uncaught unknown exception\n");
      exit_status = 2;
    }

    lua_close(l);
    ctx_g.pool->stop();
    return exit_status;
  }
}

int main(int argc, char** argv) {
  return dort::main(argc, argv);
}

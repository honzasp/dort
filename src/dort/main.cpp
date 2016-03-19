#include <thread>
#include "dort/ctx.hpp"
#include "dort/geometry.hpp"
#include "dort/lua.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_filter.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_grid.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_math.hpp"
#include "dort/lua_nbt.hpp"
#include "dort/lua_sampler.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_spectrum.hpp"
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
      luaL_requiref(l, LUA_DBLIBNAME, luaopen_debug, true);
      luaL_requiref(l, LUA_IOLIBNAME, luaopen_io, true);
      luaL_requiref(l, LUA_LOADLIBNAME, luaopen_package, true);
      luaL_requiref(l, LUA_MATHLIBNAME, luaopen_math, true);
      luaL_requiref(l, LUA_OSLIBNAME, luaopen_os, true);
      luaL_requiref(l, LUA_STRLIBNAME, luaopen_string, true);
      luaL_requiref(l, LUA_TABLIBNAME, luaopen_table, true);
      luaL_requiref(l, "zlib", luaopen_zlib, true);

      lua_getglobal(l, "package");
      lua_createtable(l, 1, 0);
      lua_pushcfunction(l, lua_searcher);
      lua_rawseti(l, -2, 1);
      lua_setfield(l, -2, "searchers");
      lua_pop(l, 1);

      auto load_sublib = [&](const char* libname, lua_CFunction open_fun) {
        std::string global_libname(std::string("dort.") + libname);
        luaL_requiref(l, global_libname.c_str(), open_fun, false);
        lua_setfield(l, -2, libname);
      };

      lua_newtable(l);
      load_sublib("builder", lua_open_builder);
      load_sublib("camera", lua_open_camera);
      load_sublib("filter", lua_open_filter);
      load_sublib("geometry", lua_open_geometry);
      load_sublib("grid", lua_open_grid);
      load_sublib("image", lua_open_image);
      load_sublib("light", lua_open_light);
      load_sublib("material", lua_open_material);
      load_sublib("math", lua_open_math);
      load_sublib("nbt", lua_open_nbt);
      load_sublib("sampler", lua_open_sampler);
      load_sublib("shape", lua_open_shape);
      load_sublib("spectrum", lua_open_spectrum);
      load_sublib("stats", lua_open_stats);
      load_sublib("texture", lua_open_texture);
      lua_setglobal(l, "dort");


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
        //stat_report_global(stderr);
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

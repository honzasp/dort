#include <thread>
#include "dort/ctx.hpp"
#include "dort/geometry.hpp"
#include "dort/lua.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_camera.hpp"
#include "dort/lua_chrono.hpp"
#include "dort/lua_env.hpp"
#include "dort/lua_filter.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_grid.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_math.hpp"
#include "dort/lua_nbt.hpp"
#include "dort/lua_render.hpp"
#include "dort/lua_sampler.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_sources.hpp"
#include "dort/lua_spectrum.hpp"
#include "dort/lua_stats.hpp"
#include "dort/lua_texture.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  int main(int argc, char** argv) {
    if(argc <= 1) {
      std::fprintf(stderr, "Usage: %s program.lua\n", argv[0]);
      return 1;
    }

    stat_init_global();

    uint32_t thread_count = std::thread::hardware_concurrency();
    //thread_count = 1; std::printf("ONE THREAD\n");
    auto pool_g = std::make_shared<ThreadPool>(max(thread_count, 1u));
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
      luaL_requiref(l, LUA_COLIBNAME, luaopen_coroutine, true);
      luaL_requiref(l, "zlib", luaopen_zlib, true);

      lua_getglobal(l, "package");
      lua_createtable(l, 2, 0);
      lua_pushcfunction(l, lua_builtin_searcher);
      lua_rawseti(l, -2, 1);
      lua_pushcfunction(l, lua_file_searcher);
      lua_rawseti(l, -2, 2);
      lua_setfield(l, -2, "searchers");
      lua_pop(l, 1);

      auto load_sublib = [&](const char* libname, lua_CFunction open_fun) {
        std::string global_libname(std::string("dort.") + libname);
        luaL_requiref(l, global_libname.c_str(), open_fun, false);
        lua_setfield(l, -2, libname);
      };

      auto load_ext = [&](const char* source_name) {
        // TODO: handle errors here
        auto source_pair = lua_sources.at(source_name);
        const char* begin = source_pair.first;
        const char* end = source_pair.second;
        int status = luaL_loadbufferx(l, begin, end - begin, source_name, "b");
        if(status != LUA_OK) {
          lua_error(l);
        }
        lua_call(l, 0, 0);
      };

      lua_newtable(l);
      load_sublib("builder", lua_open_builder);
      load_sublib("camera", lua_open_camera);
      load_sublib("chrono", lua_open_chrono);
      load_sublib("env", lua_open_env);
      load_sublib("filter", lua_open_filter);
      load_sublib("geometry", lua_open_geometry);
      load_sublib("grid", lua_open_grid);
      load_sublib("image", lua_open_image);
      load_sublib("light", lua_open_light);
      load_sublib("material", lua_open_material);
      load_sublib("math", lua_open_math);
      load_sublib("nbt", lua_open_nbt);
      load_sublib("render", lua_open_render);
      load_sublib("sampler", lua_open_sampler);
      load_sublib("shape", lua_open_shape);
      load_sublib("spectrum", lua_open_spectrum);
      load_sublib("stats", lua_open_stats);
      load_sublib("texture", lua_open_texture);
      lua_setglobal(l, "dort");

      load_ext("dort/builder_ext");
      load_ext("dort/geometry_ext");

      assert(argc >= 2);
      const char* input_file = argv[1];

      for(int32_t i = 2; i < argc; ++i) {
        ctx_g.argv.push_back(std::string(argv[i]));
      }

      lua_pushcfunction(l, [](lua_State* l) -> int {
        if(!lua_isstring(l, 1)) {
          return 1;
        }
        const char* msg = lua_tostring(l, 1);
        luaL_traceback(l, l, msg, 1);
        return 1;
      });

      int error;
      if((error = luaL_loadfile(l, input_file))) {
        std::fprintf(stderr, "Load error: %s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
        exit_status = 1;
      } else if((error = lua_pcall(l, 0, 0, -2))) {
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

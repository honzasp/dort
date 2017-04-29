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
#include "dort/lua_spectrum.hpp"
#include "dort/lua_stats.hpp"
#include "dort/lua_texture.hpp"
#include "dort/main.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  const char* const SHORT_USAGE =
R"(Usage: dort [options] [--] <program.lua> [program args]
  Options: -t <count> | -s <word> | -S <word> | -h | -v
  Use --help for longer help
)";

  const char* const LONG_USAGE = 
R"(Usage: dort [options] [--] <program.lua> [program args]
  Executes the Lua program in file <program.lua>, and passes [program args] to
  the program. [options] are:

    -t, --threads <count>
      Use <count> threads in the shared thread pool.
    -s, --enable-stats <word>
      Enable statistics matching <word>. If the argument is used at least once,
      statistics will be printed at the end of execution. This option can be
      used multiple times.
    -S, --disable-stats <word>
      Disables statistics matching <word>. This option can be used multiple
      times.

    -h, -?, --help
      Prints this message and exits.
    -v, --version
      Prints the dort version and exits.

  The exit status of the program is as follows:
    0: no errors
    1: Lua error (caused by <program.lua>)
    2: internal error (unhandled exception in dort)
    3: usage error
)";

  int main(uint32_t argc, char** argv) {
    stat_init_global();
    uint32_t thread_count = std::thread::hardware_concurrency();
    std::vector<std::string> lua_argv;
    const char* input_file = nullptr;

    bool args_ok = true;
    uint32_t arg_i = 1;
    while(arg_i < argc) {
      std::string arg(argv[arg_i]);
      if(arg == "--") {
        arg_i += 1; break;
      } else if(arg == "-t" || arg == "--threads") {
        if(arg_i + 1 >= argc) { args_ok = false; break; }
        if(std::sscanf(argv[arg_i + 1], "%u", &thread_count) != 1) {
          args_ok = false; break;
        }
        arg_i += 2;
      } else if(arg == "-s" || arg == "--enable-stats") {
        if(arg_i + 1 >= argc) { args_ok = false; break; }
        stat_enable(argv[arg_i + 1], true);
        arg_i += 2;
      } else if(arg == "-S" || arg == "--disable-stats") {
        if(arg_i + 1 >= argc) { args_ok = false; break; }
        stat_enable(argv[arg_i + 1], false);
        arg_i += 2;
      } else if(arg == "-h" || arg == "-?" || arg == "--help") {
        std::fprintf(stdout, "%s", LONG_USAGE);
        return 0;
      } else if(arg == "-v" || arg == "--version") {
        std::fprintf(stdout, "dort\n");
        return 0;
      } else if(arg.size() >= 1 && arg.at(0) == '-') {
        args_ok = false; break;
      } else {
        break;
      }
    }

    if(arg_i < argc) {
      input_file = argv[arg_i++];
    } else {
      args_ok = false;
    }
    for(; arg_i < argc; ++arg_i) {
      lua_argv.push_back(argv[arg_i]);
    }

    if(!args_ok) {
      std::fprintf(stderr, "%s", SHORT_USAGE);
      return 3;
    }

    stat_init_thread();
    auto pool_g = std::make_shared<ThreadPool>(max(thread_count, 1u));
    CtxG ctx_g(pool_g);
    ctx_g.argv = std::move(lua_argv);

    lua_State* l = luaL_newstate();
    lua_set_ctx(l, &ctx_g);
    
    int exit_status;
    try {
      exit_status = main_lua(l, input_file);
      ctx_g.pool->stop();
      stat_report_global(stderr);
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

  int main_lua(lua_State* l, const char* input_file) {
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
      return 1;
    } else if((error = lua_pcall(l, 0, 0, -2))) {
      std::fprintf(stderr, "Runtime error: %s\n", lua_tostring(l, -1));
      lua_pop(l, 1);
      return 1;
    } else {
      return 0;
    }
  }
}

int main(int argc, char** argv) {
  return dort::main(argc, argv);
}

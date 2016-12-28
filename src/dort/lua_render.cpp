#ifdef DORT_USE_GTK
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#endif
#include "dort/bdpt_renderer.hpp"
#include "dort/direct_renderer.hpp"
#include "dort/dot_renderer.hpp"
#include "dort/film.hpp"
#include "dort/filter.hpp"
#include "dort/igi_renderer.hpp"
#include "dort/lua.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/lua_render.hpp"
#include "dort/path_renderer.hpp"
#include "dort/random_sampler.hpp"
#include "dort/sppm_renderer.hpp"
#include "dort/stats.hpp"

namespace dort {
  int lua_open_render(lua_State* l) {
    const luaL_Reg render_job_methods[] = {
      {"__gc", lua_gc_shared_obj<RenderJob, RENDER_JOB_TNAME>},
      {0, 0},
    };

    const luaL_Reg render_funs[] = {
      {"make", lua_render_make},
      {"render_sync", lua_render_render_sync},
      {"render_async", lua_render_render_async},
      {"image_to_pixbuf", lua_render_image_to_pixbuf},
      {"get_preview", lua_render_get_preview},
      {"get_image", lua_render_get_image},
      {"get_progress", lua_render_get_progress},
      {0, 0},
    };

    lua_register_type(l, RENDER_JOB_TNAME, render_job_methods);
    luaL_newlib(l, render_funs);
    return 1;
  }

  int lua_render_make(lua_State* l) {
    auto scene = lua_check_scene(l, 1);

    int p = 2;
    uint32_t x_res = lua_param_uint32_opt(l, p, "x_res", 800);
    uint32_t y_res = lua_param_uint32_opt(l, p, "y_res", 600);
    auto filter = lua_param_filter_opt(l, p, "filter", 
        std::make_shared<BoxFilter>(Vec2(0.5f, 0.5f)));
    auto sampler = lua_param_sampler_opt(l, p, "sampler",
        std::make_shared<RandomSampler>(1, 42));
    auto method = lua_param_string_opt(l, p, "renderer", "direct");

    auto film = std::make_shared<Film>(x_res, y_res, filter);

    std::shared_ptr<Renderer> renderer;
    if(method == "direct") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      renderer = std::make_shared<DirectRenderer>(
          scene, film, sampler, max_depth);
    } else if(method == "dot") {
      renderer = std::make_shared<DotRenderer>(scene, film, sampler);
    } else if(method == "pt" || method == "path") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      renderer = std::make_shared<PathRenderer>(
          scene, film, sampler, max_depth);
    } else if(method == "bdpt") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      renderer = std::make_shared<BdptRenderer>(
          scene, film, sampler, max_depth);
    } else if(method == "igi") {
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      uint32_t max_light_depth = lua_param_uint32_opt(l, p, "max_light_depth", 5);
      uint32_t light_set_count = lua_param_uint32_opt(l, p, "light_sets", 1);
      uint32_t path_count = lua_param_uint32_opt(l, p, "light_paths", 32);
      float g_limit = lua_param_float_opt(l, p, "g_limit", 5.f);
      float roulette_threshold = lua_param_float_opt(l, p, 
          "roulette_threshold", 0.001f);
      renderer = std::make_shared<IgiRenderer>(
          scene, film, sampler,
          max_depth, max_light_depth, light_set_count, path_count,
          g_limit, roulette_threshold);
    } else if(method == "sppm") {
      float initial_radius = lua_param_float(l, p, "initial_radius");
      uint32_t iteration_count = lua_param_uint32(l, p, "iterations");
      uint32_t max_depth = lua_param_uint32_opt(l, p, "max_depth", 5);
      uint32_t max_photon_depth = lua_param_uint32_opt(l, p, "max_light_depth", 5);
      uint32_t photon_path_count = lua_param_uint32_opt(l, p, "light_paths", 32);
      float alpha = lua_param_float_opt(l, p, "alpha", 0.7f);

      std::string mode_str = lua_param_string_opt(l, p, "parallel_mode", "automatic");
      SppmRenderer::ParallelMode parallel_mode;
      if(mode_str == "automatic") {
        parallel_mode = SppmRenderer::ParallelMode::Automatic;
      } else if(mode_str == "serial_iterations") {
        parallel_mode = SppmRenderer::ParallelMode::SerialIterations;
      } else if(mode_str == "parallel_iterations") {
        parallel_mode = SppmRenderer::ParallelMode::ParallelIterations;
      } else {
        return luaL_error(l, "Unrecognized SPPM parallel mode: %s", mode_str.c_str());
      }

      renderer = std::make_shared<SppmRenderer>(
          scene, film, sampler, initial_radius, iteration_count,
          max_depth, max_photon_depth, photon_path_count, 
          alpha, parallel_mode);
    } else {
      return luaL_error(l, "Unrecognized rendering method: %s", method.c_str());
    }
    lua_params_check_unused(l, p);

    auto render_job = std::make_shared<RenderJob>();
    render_job->l = l;
    render_job->lua_id = std::this_thread::get_id();
    render_job->renderer = renderer;
    render_job->film = film;
    render_job->render_started = false;
    render_job->render_finished = false;
    lua_push_render_job(l, render_job);
    return 1;
  }

  int lua_render_render_sync(lua_State* l) {
    auto render_job = lua_check_render_job(l, 1);
    if(render_job->render_started) {
      return luaL_error(l, "This render job has already started");
    }

    render_job->render_started = true;
    render_job->renderer->render(*lua_get_ctx(l));
    render_job->render_finished = true;
    return 0;
  }

#ifdef DORT_USE_GTK
  int lua_render_render_async(lua_State* l) {
    auto render_job = lua_check_render_job(l, 1);
    if(render_job->render_started) {
      return luaL_error(l, "This render job has already started");
    }
    assert(!render_job->async_thread.joinable());
    assert(!render_job->self_ptr);

    if(!lua_isfunction(l, 2)) {
      return luaL_error(l, "A callback function is required as argument");
    }

    // save the callback in the Lua registry, use the render_job pointer as key
    lua_pushlightuserdata(l, render_job.get());
    lua_pushvalue(l, 2);
    lua_settable(l, LUA_REGISTRYINDEX);

    auto ctx = lua_get_ctx(l);
    render_job->render_started = true;
    // initialize the GTask that will be used to fire the callback from the Gtk
    // event loop later. the self_ptr serves to keep the RenderJob alive until
    // the GTask callback fires (the GTask stores a raw pointer to the
    // RenderJob)
    render_job->self_ptr = render_job;
    render_job->result_gtask = g_task_new(nullptr, nullptr,
        RenderJob::result_callback, render_job.get());
    render_job->async_thread = std::thread([ctx, render_job]() {
      stat_init_thread();
      render_job->renderer->render(*ctx);
      render_job->schedule_result_callback();
      stat_finish_thread();
    });

    return 0;
  }

  RenderJob::~RenderJob() {
    assert(!this->async_thread.joinable());
    g_clear_object(&this->result_gtask);
  }

  void RenderJob::schedule_result_callback() {
    // this is called from the main rendering thread
    assert(std::this_thread::get_id() != this->lua_id);
    // ensure that the GTask callback will be called from the Gtk main loop
    g_task_return_pointer(this->result_gtask, nullptr, nullptr);
  }

  void RenderJob::result_callback(GObject*, GAsyncResult*, gpointer render_job_ptr) {
    // this is called back on the Lua thread from the Gtk main loop
    auto render_job = static_cast<RenderJob*>(render_job_ptr)->self_ptr;
    assert(std::this_thread::get_id() == render_job->lua_id);

    render_job->async_thread.join();
    render_job->self_ptr.reset();
    render_job->render_finished = true;

    // i am not sure how safe it is to call Lua API from inside the Gtk loop
    // (which is itself running inside Lua), but it seems to be ok.

    // get the callback
    lua_State* l = render_job->l;
    lua_pushlightuserdata(l, render_job.get());
    lua_gettable(l, LUA_REGISTRYINDEX);

    // but remove the reference from the registry (otherwise the callback would
    // leak, persisting in the reference forever)
    lua_pushlightuserdata(l, render_job.get());
    lua_pushnil(l);
    lua_settable(l, LUA_REGISTRYINDEX);

    lua_call(l, 0, 0);
  }

  int lua_render_image_to_pixbuf(lua_State* l) {
    auto image = lua_check_image_8(l, 1);
    uint32_t x_res = image->x_res;
    uint32_t y_res = image->y_res;
    GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, x_res, y_res);
    guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
    uint32_t rowstride = gdk_pixbuf_get_rowstride(pixbuf);

    for(uint32_t y = 0; y < image->y_res; ++y) {
      for(uint32_t x = 0; x < image->x_res; ++x) {
        PixelRgb8 pixel = image->get_pixel(x, y);
        guchar* pixel_ptr = pixels + rowstride*y + 3*x;
        pixel_ptr[0] = pixel.r;
        pixel_ptr[1] = pixel.g;
        pixel_ptr[2] = pixel.b;
      }
    }

    lua_pushlightuserdata(l, pixbuf);
    return 1;
  }
#else
  int lua_render_render_async(lua_State* l) {
    return luaL_error(l, "Asynchronous renders are supported only "
        "with Gtk event loop, but this dort binary was compiled without Gtk");
  }
  int lua_render_image_to_pixbuf(lua_State* l) {
    return luaL_error(l, "This dort binary was compiled without Gtk");
  }
#endif

  int lua_render_get_preview(lua_State* l) {
    auto render_job = lua_check_render_job(l, 1);

    // TODO: the film is concurrently accessed by the renderer, so this is one
    // big data race. However, this is a quick'n'dirty solution that should not
    // cause any memory unsafety.
    auto film = render_job->film;
    auto image = render_job->film->to_image<PixelRgb8>();
    lua_push_image_8(l, std::make_shared<Image<PixelRgb8>>(std::move(image)));
    return 1;
  }

  int lua_render_get_progress(lua_State* l) {
    // TODO: return something reasonable
    auto render_job = lua_check_render_job(l, 1);
    lua_pushnumber(l, 0.4f);
    return 1;
  }

  int lua_render_get_image(lua_State* l) {
    auto render_job = lua_check_render_job(l, 1);
    if(!render_job->render_finished) {
      return luaL_error(l, render_job->render_started 
          ? "Render has not finished yet" : "Render has not been started");
    }

    int p = 2;
    bool hdr = lua_param_bool_opt(l, p, "hdr", false);
    lua_params_check_unused(l, p);

    auto film = render_job->film;
    if(hdr) {
      auto image = std::make_shared<Image<PixelRgbFloat>>(
          film->to_image<PixelRgbFloat>());
      lua_push_image_f(l, image);
    } else {
      auto image = std::make_shared<Image<PixelRgb8>>(
          film->to_image<PixelRgb8>());
      lua_push_image_8(l, image);
    }
    return 1;
  }


  std::shared_ptr<RenderJob> lua_check_render_job(lua_State* l, int idx) {
    return lua_check_shared_obj<RenderJob, RENDER_JOB_TNAME>(l, idx);
  }
  bool lua_test_render_job(lua_State* l, int idx) {
    return lua_test_shared_obj<RenderJob, RENDER_JOB_TNAME>(l, idx);
  }
  void lua_push_render_job(lua_State* l, std::shared_ptr<RenderJob> job) {
    lua_push_shared_obj<RenderJob, RENDER_JOB_TNAME>(l, job);
  }
}

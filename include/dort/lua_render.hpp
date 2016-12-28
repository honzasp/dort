#pragma once
#ifdef DORT_USE_GTK
#include <gio/gio.h>
#endif
#include <thread>
#include "dort/dort.hpp"
#include "dort/lua.hpp"

namespace dort {
  constexpr const char RENDER_JOB_TNAME[] = "dort.RenderJob";

  struct RenderJob {
    RenderJob(RenderJob&&) = delete;
    RenderJob() = default;

    lua_State* l;
    std::thread::id lua_id;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<Film> film;
    bool render_started;
    bool render_finished;
#ifdef DORT_USE_GTK
    std::shared_ptr<RenderJob> self_ptr;
    GTask* result_gtask = nullptr;
    std::thread async_thread;

    ~RenderJob();
    void schedule_result_callback();
    static void result_callback(GObject*, GAsyncResult*, gpointer render_job_ptr);
#endif
  };

  int lua_open_render(lua_State* l);

  int lua_render_make(lua_State* l);
  int lua_render_render_sync(lua_State* l);
  int lua_render_render_async(lua_State* l);
  int lua_render_image_to_pixbuf(lua_State* l);
  int lua_render_get_preview(lua_State* l);
  int lua_render_get_progress(lua_State* l);
  int lua_render_get_image(lua_State* l);

  std::shared_ptr<RenderJob> lua_check_render_job(lua_State* l, int idx);
  bool lua_test_render_job(lua_State* l, int idx);
  void lua_push_render_job(lua_State* l, std::shared_ptr<RenderJob> render_job);
}

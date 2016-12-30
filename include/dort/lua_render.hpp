#pragma once
#ifdef DORT_USE_GTK
#include <gio/gio.h>
#endif
#include <atomic>
#include <thread>
#include "dort/atomic_float.hpp"
#include "dort/lua.hpp"
#include "dort/renderer.hpp"

namespace dort {
  constexpr const char RENDER_JOB_TNAME[] = "dort.RenderJob";

  struct RenderJob {
    struct JobProgress;
    RenderJob(RenderJob&&) = delete;
    RenderJob() = default;

    lua_State* l;
    std::thread::id lua_id;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<Film> film;
    std::shared_ptr<JobProgress> progress;
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
    
    struct JobProgress final: public Progress {
      std::atomic<bool> cancelled =  { false };
      atomic_float percent_done = { 0.f };

      virtual bool is_cancelled() const override final { 
        return this->cancelled.load(std::memory_order_relaxed);
      }
      virtual void set_percent_done(float percent) override final {
        this->percent_done.store(percent, std::memory_order_relaxed);
      }
    };
  };

  int lua_open_render(lua_State* l);

  int lua_render_make(lua_State* l);
  int lua_render_render_sync(lua_State* l);
  int lua_render_render_async(lua_State* l);
  int lua_render_image_to_pixbuf(lua_State* l);
  int lua_render_cancel(lua_State* l);
  int lua_render_get_preview(lua_State* l);
  int lua_render_get_progress(lua_State* l);
  int lua_render_get_image(lua_State* l);

  std::shared_ptr<RenderJob> lua_check_render_job(lua_State* l, int idx);
  bool lua_test_render_job(lua_State* l, int idx);
  void lua_push_render_job(lua_State* l, std::shared_ptr<RenderJob> render_job);
}

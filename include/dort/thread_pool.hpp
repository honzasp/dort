#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  class ThreadPool {
    struct Loop {
      std::function<void(uint32_t)> fun;
      uint32_t count;
      std::atomic<uint32_t> started_count { 0 };
      std::atomic<uint32_t> finished_count { 0 };
      bool removed_flag = false;
      bool finished_flag = false;
      std::condition_variable finished_condvar;
    };

    std::mutex mutex;
    bool stop_flag;
    std::vector<std::thread> workers;
    std::condition_variable wakeup_condvar;
    std::list<Loop*> loop_stack;
  public:
    ThreadPool();
    ~ThreadPool();

    void start(uint32_t thread_count);
    void stop();
    void restart();
    uint32_t thread_count();

    void loop(uint32_t count, std::function<void(uint32_t)> fun);
  private:
    static void worker_body(ThreadPool& pool);
    static void work_on_loop(ThreadPool& pool, Loop& loop,
        std::list<Loop*>::iterator loop_iter, std::unique_lock<std::mutex>& lock,
        bool wakeup_others);
  };

  template<class F>
  void parallel_for(ThreadPool& pool, uint32_t count, F fun) {
    pool.loop(count, fun);
  }

  template<class F>
  void parallel_for_or_serial(ThreadPool& pool, bool serial, uint32_t count, F fun) {
    if(serial) {
      for(uint32_t i = 0; i < count; ++i) {
        fun(i);
      }
    } else {
      pool.loop(count, fun);
    }
  }
}

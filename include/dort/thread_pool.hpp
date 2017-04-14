#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  class ThreadPool {
    std::thread::id main_thread_id;
    std::vector<std::thread> threads;
    std::deque<std::function<void()>> queue;
    std::mutex queue_mutex;
    std::condition_variable queue_condvar;
    std::atomic<bool> stop_flag;
  public:
    ThreadPool(uint32_t thread_count = 0);
    ~ThreadPool();
    void schedule(std::function<void()> job);
    void start(uint32_t thread_count);
    void stop();
    void restart();
    uint32_t thread_count() const;
  private:
    void thread_body();
  };

  void fork_join(ThreadPool& pool, uint32_t count,
      std::function<void(uint32_t)> worker);
  void fork_join_or_serial(ThreadPool& pool, bool serial,
      uint32_t count, std::function<void(uint32_t)> worker);
}

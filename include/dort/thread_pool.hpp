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
    ThreadPool(uint32_t num_threads);
    ~ThreadPool();
    void schedule(std::function<void()> job);
    void stop();
    uint32_t num_threads() const;
  private:
    void thread_body();
  };

  void fork_join(ThreadPool& pool, uint32_t count,
      std::function<void(uint32_t)> worker);
}

#include <future>
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  ThreadPool::ThreadPool(uint32_t thread_count) {
    this->main_thread_id = std::this_thread::get_id();
    this->start(thread_count);
  }

  ThreadPool::~ThreadPool() {
    this->stop();
  }

  void ThreadPool::schedule(std::function<void()> job) {
    assert(!this->threads.empty());
    assert(!this->stop_flag.load());

    StatTimer t(TIMER_POOL_SCHEDULE);
    stat_count(COUNTER_POOL_JOBS);
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    this->queue.push_back(std::move(job));
    lock.unlock();
    this->queue_condvar.notify_one();
  }

  void ThreadPool::start(uint32_t thread_count) {
    assert(this->main_thread_id == std::this_thread::get_id());
    assert(this->threads.empty());
    this->stop_flag.store(false);
    for(uint32_t i = 0; i < thread_count; ++i) {
      this->threads.push_back(std::thread(&ThreadPool::thread_body, this));
    }
  }

  void ThreadPool::stop() {
    if(this->stop_flag.load()) {
      return;
    }
    assert(this->main_thread_id == std::this_thread::get_id());

    this->stop_flag.store(true);
    this->queue_condvar.notify_all();
    for(auto& thread: this->threads) {
      thread.join();
    }
    this->threads.clear();
  }

  void ThreadPool::restart() {
    uint32_t thread_count = this->thread_count();
    this->stop();
    this->start(thread_count);
  }

  void ThreadPool::thread_body() {
    stat_init_thread();

    for(;;) {
      std::unique_lock<std::mutex> lock(this->queue_mutex);
      if(this->queue.empty()) {
        if(this->stop_flag.load()) {
          break;
        }

        StatTimer t(TIMER_POOL_WAIT);
        stat_count(COUNTER_POOL_WAITS);
        this->queue_condvar.wait(lock);
      } else {
        stat_count(COUNTER_POOL_NO_WAITS);
      }

      if(!this->queue.empty()) {
        auto job = std::move(this->queue.front());
        this->queue.pop_front();
        lock.unlock();

        StatTimer t(TIMER_POOL_JOB);
        job();
      }
    }

    stat_finish_thread();
  }

  uint32_t ThreadPool::thread_count() const {
    return this->threads.size();
  }

  void fork_join(ThreadPool& pool, uint32_t count,
      std::function<void(uint32_t)> worker)
  {
    std::atomic<uint32_t> done_jobs(0);
    std::promise<void> done;

    for(uint32_t job = 0; job < count; ++job) {
      pool.schedule([&, job]() {
        worker(job);
        if(done_jobs.fetch_add(1) + 1 >= count) {
          done.set_value();
        }
      });
    }

    done.get_future().get();
  }

  void fork_join_or_serial(ThreadPool& pool, bool serial,
      uint32_t count, std::function<void(uint32_t)> worker)
  {
    if(serial) {
      for(uint32_t job = 0; job < count; ++job) {
        worker(job);
      }
    } else {
      fork_join(pool, count, worker);
    }
  }
}

#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  ThreadPool::ThreadPool(uint32_t num_threads) {
    this->stop_flag.store(false);
    for(uint32_t i = 0; i < num_threads; ++i) {
      this->threads.push_back(std::thread(&ThreadPool::thread_body, this));
    }
  }

  ThreadPool::~ThreadPool() {
    this->stop();
  }

  void ThreadPool::schedule(std::function<void()> job) {
    assert(!this->threads.empty());
    std::unique_lock<std::mutex> lock(this->queue_mutex);
    this->queue.push_back(std::move(job));
    lock.unlock();
    this->queue_condvar.notify_one();
  }

  void ThreadPool::stop() {
    this->stop_flag.store(true);
    this->queue_condvar.notify_all();
    for(auto& thread: this->threads) {
      thread.join();
    }
    this->threads.clear();
  }

  void ThreadPool::thread_body() {
    stat_init_thread();

    for(;;) {
      std::unique_lock<std::mutex> lock(this->queue_mutex);
      if(this->queue.empty()) {
        if(this->stop_flag.load()) {
          break;
        }
        this->queue_condvar.wait(lock);
      }

      if(!this->queue.empty()) {
        auto job = this->queue.front();
        this->queue.pop_front();
        lock.unlock();
        job();
      }
    }

    stat_finish_thread();
  }

  uint32_t ThreadPool::num_threads() const {
    return this->threads.size();
  }

  void fork_join(ThreadPool& pool, uint32_t count,
      std::function<void(uint32_t)> worker)
  {
    uint32_t done_jobs = 0;
    std::mutex done_mutex;
    std::condition_variable done_condvar;

    for(uint32_t job = 0; job < count; ++job) {
      pool.schedule([&, job]() {
        worker(job);
        std::unique_lock<std::mutex> lock(done_mutex);
        done_jobs += 1;
        lock.unlock();
        done_condvar.notify_one();
      });
    }

    for(;;) {
      std::unique_lock<std::mutex> lock(done_mutex);
      if(done_jobs >= count) {
        return;
      }
      done_condvar.wait(lock);
    }
  }
}

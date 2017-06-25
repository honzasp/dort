#include <future>
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  ThreadPool::ThreadPool() {
    this->stop_flag = false;
  }

  ThreadPool::~ThreadPool() {
    this->stop();
  }

  void ThreadPool::start(uint32_t thread_count) {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->stop_flag = false;
    while(this->workers.size() + 1 < thread_count) {
      this->workers.push_back(std::thread([this]() {
        ThreadPool::worker_body(*this); 
      }));
    }
  }

  void ThreadPool::stop() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->stop_flag = true;
    this->wakeup_condvar.notify_all();

    while(!this->workers.empty()) {
      auto worker = std::move(this->workers.back());
      this->workers.pop_back();
      lock.unlock();
      worker.join();
      lock.lock();
    }
  }

  void ThreadPool::restart() {
    uint32_t thread_count = this->thread_count();
    this->stop();
    this->start(thread_count);
  }

  uint32_t ThreadPool::thread_count() {
    std::unique_lock<std::mutex> lock(this->mutex);
    return this->workers.size() + 1;
  }

  void ThreadPool::loop(uint32_t count, std::function<void(uint32_t)> fun) {
    Loop loop;
    loop.fun = fun;
    loop.count = count;

    // add the loop to the stack and work on it until there are no iterations
    // left
    std::unique_lock<std::mutex> lock(this->mutex);
    this->loop_stack.push_front(&loop);
    auto loop_iter = this->loop_stack.begin();
    work_on_loop(*this, loop, loop_iter, lock, true);
    assert(loop.removed_flag);

    // wait for the loop to finish, possibly doing other work in the meantime
    while(!loop.finished_flag) {
      if(this->loop_stack.empty()) {
        stat_count(COUNTER_POOL_WAITS);
        StatTimer t(TIMER_POOL_WAIT);
        loop.finished_condvar.wait(lock);
      } else {
        auto loop_iter = this->loop_stack.begin();
        ThreadPool::work_on_loop(*this, **loop_iter, loop_iter, lock, false);
      }
    }
  }

  void ThreadPool::worker_body(ThreadPool& pool) {
    stat_init_thread();

    std::unique_lock<std::mutex> lock(pool.mutex);
    for(;;) {
      assert(lock.owns_lock());
      if(pool.stop_flag && pool.loop_stack.empty()) {
        break;
      } if(pool.loop_stack.empty()) {
        stat_count(COUNTER_POOL_WAITS);
        StatTimer t(TIMER_POOL_WAIT);
        pool.wakeup_condvar.wait(lock);
      } else {
        stat_count(COUNTER_POOL_NO_WAITS);
      }

      if(!pool.loop_stack.empty()) {
        auto loop_iter = pool.loop_stack.begin();
        ThreadPool::work_on_loop(pool, **loop_iter, loop_iter, lock, false);
      }
    }

    stat_finish_thread();
  }

  void ThreadPool::work_on_loop(ThreadPool& pool, Loop& loop,
      std::list<Loop*>::iterator loop_iter, std::unique_lock<std::mutex>& lock,
      bool wakeup_others)
  {
    lock.unlock();
    if(wakeup_others) {
      pool.wakeup_condvar.notify_all();
    }

    StatTimer work_timer(TIMER_POOL_WORK);
    auto fun = loop.fun;
    uint32_t count = loop.count;
    for(;;) {
      // perform iterations until the loop is emptied (the fast path needs no
      // locking)
      uint32_t iter = loop.started_count.fetch_add(1);
      if(iter < count) {
        StatTimer job_timer(TIMER_POOL_JOB);
        fun(iter);
      } else {
        work_timer.stop();
        lock.lock();
        break;
      }

      uint32_t finished_count = loop.finished_count.fetch_add(1) + 1;
      assert(finished_count <= count);
      if(finished_count >= count) {
        // we finished the last iteration of the loop, so we signal that the
        // loop is finished, possibly waking up the thread that waits for the
        // loop to finish.
        work_timer.stop();
        lock.lock();
        if(!loop.finished_flag) {
          loop.finished_flag = true;
          loop.finished_condvar.notify_all();
        }
        break;
      }
    }

    assert(lock.owns_lock());
    // the loop has no iterations left, we will race to remove the loop from the
    // stack. note that the loop may not be finished, other threads may still
    // work on the last iterations.
    if(!loop.removed_flag) {
      loop.removed_flag = true;
      pool.loop_stack.erase(loop_iter);
    }
    return;
  }
}

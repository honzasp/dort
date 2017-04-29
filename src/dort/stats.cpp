#ifndef DORT_DISABLE_STAT
#include <cinttypes>
#include <mutex>
#include <random>
#include "dort/math.hpp"
#include "dort/stats.hpp"

namespace dort {
  Stats GLOBAL_STATS;
  std::mutex GLOBAL_STATS_MUTEX;
  thread_local Stats THREAD_STATS;
  thread_local std::ranlux24_base THREAD_TIME_SAMPLE_RNG;

  const std::vector<StatCounterDef> STAT_COUNTER_DEFS = {
    { "scene isect" },
    { "scene isect_p" },
    { "bvh fast_box_isect_p" },
    { "bvh fast_box_isect_p hit" },
    { "bvh prim isect" },
    { "bvh prim isect hit" },
    { "bvh prim isect_p" },
    { "bvh prim isect_p hit" },
    { "triangle hit" },
    { "triangle hit hit" },
    { "triangle hit_p" },
    { "triangle hit_p hit" },
    { "pool jobs" },
    { "pool no-waits" },
    { "pool waits" },
  };

  const std::vector<StatDistribIntDef> STAT_DISTRIB_INT_DEFS = {
    { "bvh traverse count" },
    { "bvh build_node parallel count" },
    { "bvh build_node serial count" },
    { "bvh build serial count" },
    { "bvh build linear resize" },
    { "bvh split_middle jobs" },
    { "bsdf number of bxdfs" },
    { "render jobs" },
  };

  const std::vector<StatDistribTimeDef> STAT_DISTRIB_TIME_DEFS = {
    { "render", UINT32_MAX },
    { "render tile", UINT32_MAX / 16 },
    { "direct get_radiance", UINT32_MAX / 256 },
    { "uniform_sample_all_lights", UINT32_MAX / 256 }, 
    { "estimate_direct", UINT32_MAX / 512 },
    { "trace_specular", UINT32_MAX / 512 },
    { "scene isect", UINT32_MAX / 256 },
    { "scene isect_p", UINT32_MAX / 256 },
    { "film add_sample", UINT32_MAX / 256 },
    { "film add_splat", UINT32_MAX / 256 },
    { "film add_tile", UINT32_MAX / 16 },
    { "rng float", UINT32_MAX / 1024 },
    { "rng uint32", UINT32_MAX / 1024 },
    { "bvh build", UINT32_MAX },
    { "bvh compute build_infos", UINT32_MAX },
    { "bvh build node parallel", UINT32_MAX },
    { "bvh build node serial", UINT32_MAX / 256 },
    { "bvh build partition parallel", UINT32_MAX },
    { "bvh build partition serial", UINT32_MAX / 256 },
    { "bvh write_linear_node", UINT32_MAX / 1024 },
    { "bvh write_linear_node resize", UINT32_MAX },
    { "bvh split_middle", UINT32_MAX / 256 },
    { "bvh split_middle bounds out parallel", UINT32_MAX },
    { "bvh split_middle bounds out serial", UINT32_MAX / 256 },
    { "bvh split_middle bounds in parallel", UINT32_MAX },
    { "bvh split_middle bounds in serial", UINT32_MAX / 1024 },
    { "bvh split_median", UINT32_MAX / 256 },
    { "bvh split_sah", UINT32_MAX / 256 },
    { "bvh traverse node", UINT32_MAX / 256 },
    { "bvh traverse elem", UINT32_MAX / 256 },
    { "pool wait", UINT32_MAX / 16 },
    { "pool job", UINT32_MAX / 16 },
    { "pool schedule", UINT32_MAX / 16 },
  };

  int64_t stat_clock_now_ns() {
    struct timespec tp;
    ::clock_gettime(CLOCK_MONOTONIC, &tp);
    return 1000ll*1000ll*1000ll * int64_t(tp.tv_sec) + int64_t(tp.tv_nsec);
  }

  void stat_init_global() {
    stat_init(GLOBAL_STATS);
  }

  void stat_init_thread() {
    assert(GLOBAL_STATS.initialized);
    stat_init(THREAD_STATS);

    THREAD_STATS.enabled = GLOBAL_STATS.enabled;
    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      THREAD_STATS.counters.at(i).enabled = GLOBAL_STATS.counters.at(i).enabled;
    }
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      THREAD_STATS.distrib_ints.at(i).enabled = GLOBAL_STATS.distrib_ints.at(i).enabled;
    }
    for(uint32_t i = 0; i < _TIMER_END; ++i) {
      THREAD_STATS.distrib_times.at(i).enabled = GLOBAL_STATS.distrib_times.at(i).enabled;
    }
  }

  void stat_init(Stats& stats) {
    assert(!stats.initialized);
    stats.counters = std::vector<Stats::Counter>(_COUNTER_END);
    stats.distrib_ints = std::vector<Stats::DistribInt>(_DISTRIB_INT_END);
    stats.distrib_times = std::vector<Stats::DistribTime>(_TIMER_END);
    stats.initialized = true;
    stats.enabled = false;
  }

  void stat_finish_thread() {
    assert(THREAD_STATS.initialized);
    assert(GLOBAL_STATS.initialized);

    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    if(!GLOBAL_STATS.enabled) { return; }

    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      GLOBAL_STATS.counters.at(i).count += THREAD_STATS.counters.at(i).count;
    }
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      auto& glob = GLOBAL_STATS.distrib_ints.at(i);
      auto& thrd = THREAD_STATS.distrib_ints.at(i);
      glob.count += thrd.count;
      glob.sum += thrd.sum;
      glob.sum_squares += thrd.sum_squares;
      glob.min = std::min(glob.min, thrd.min);
      glob.max = std::max(glob.max, thrd.max);
    }
    for(uint32_t i = 0; i < _TIMER_END; ++i) {
      auto& glob = GLOBAL_STATS.distrib_times.at(i);
      auto& thrd = THREAD_STATS.distrib_times.at(i);
      glob.total_count += thrd.total_count;
      glob.sampled_count += thrd.sampled_count;
      glob.sum_ns += thrd.sum_ns;
      glob.sum_squares_ns += thrd.sum_squares_ns;
      glob.min_ns = std::min(glob.min_ns, thrd.min_ns);
      glob.max_ns = std::max(glob.max_ns, thrd.max_ns);
    }
    {
      auto& glob = GLOBAL_STATS.timer_overhead_distrib;
      auto& thrd = THREAD_STATS.timer_overhead_distrib;
      glob.sampled_count += thrd.sampled_count;
      glob.sum_ns += thrd.sum_ns;
    }

    THREAD_STATS.initialized = false;
  }

  void stat_report_global(FILE* output) {
    assert(GLOBAL_STATS.initialized);
    stat_finish_thread();

    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    if(!GLOBAL_STATS.enabled) { return; }
    std::fprintf(output, "# Dort statistics\n");

    std::fprintf(output, "## Counters\n");
    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      const auto& counter = GLOBAL_STATS.counters.at(i);
      if(!counter.enabled) { continue; }
      std::fprintf(output, "- %-30s  %10" PRIu64 "\n", 
          STAT_COUNTER_DEFS.at(i).name, counter.count);
    }

    std::fprintf(output, "## Integer distributions\n");
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      const auto& distrib = GLOBAL_STATS.distrib_ints.at(i);
      if(!distrib.enabled) { continue; }

      std::fprintf(output, "- %-30s  ", STAT_DISTRIB_INT_DEFS.at(i).name);
      if(distrib.count == 0) {
        std::fprintf(output, "(no samples)\n");
        continue;
      }

      float average = float(distrib.sum) / float(distrib.count);
      float average_square = float(distrib.sum_squares) / float(distrib.count);
      float variance = abs(average_square - average * average);
      float stddev = sqrt(variance);
      std::fprintf(output, "avg %g, sd %g, min %" PRIi64 ", max %" PRIi64
          ", n %" PRIu64 "\n", average, stddev, distrib.min, distrib.max,
          distrib.count);
    }

    std::fprintf(output, "## Time distributions\n");
    float estimate_overhead_ns = 0.f;
    const auto& overhead_distrib = GLOBAL_STATS.timer_overhead_distrib;
    if(overhead_distrib.sampled_count > 0) {
      estimate_overhead_ns = float(overhead_distrib.sum_ns) 
        / float(overhead_distrib.sampled_count);
      std::fprintf(output, "estimated overhead %3g ns (%" PRIu64 ")\n",
          estimate_overhead_ns, overhead_distrib.sampled_count);
    }

    for(uint32_t i = 0; i < _TIMER_END; ++i) {
      const auto& distrib = GLOBAL_STATS.distrib_times.at(i);
      if(!distrib.enabled) { continue; }

      std::fprintf(output, "- %-30s  ", STAT_DISTRIB_TIME_DEFS.at(i).name);
      if(distrib.sampled_count == 0) {
        std::fprintf(output, "(no samples)\n");
        continue;
      }

      auto p = [&](float time_ns) {
        if(abs(time_ns) < 20e3f) {
          std::fprintf(output, "%3g ns", time_ns);
        } else if(abs(time_ns) < 20e6f) {
          std::fprintf(output, "%3g us", time_ns * 1e-3f);
        } else if(abs(time_ns) < 20e9f) {
          std::fprintf(output, "%3g ms", time_ns * 1e-6f);
        } else if(abs(time_ns) < 2000e12f) {
          std::fprintf(output, "%3g s", time_ns * 1e-9f);
        } else {
          std::fprintf(output, "%3g min", time_ns * 1e-9f / 60.f);
        }
      };

      float average_ns = float(distrib.sum_ns) 
        / float(distrib.sampled_count);
      float average_square_ns = float(distrib.sum_squares_ns) 
        / float(distrib.sampled_count);
      float estimate_total_ns = (average_ns - estimate_overhead_ns) 
        * float(distrib.total_count);
      float variance = abs(average_square_ns - average_ns * average_ns);
      float stddev_ns = sqrt(variance);

      std::fprintf(output, "avg "); p(average_ns);
      std::fprintf(output, ", sd "); p(stddev_ns);
      std::fprintf(output, ", n %" PRIu64 "/%" PRIu64,
          distrib.sampled_count, distrib.total_count);
      std::fprintf(output, "\n                                  ");
      std::fprintf(output, "total ~"); p(estimate_total_ns);
      std::fprintf(output, ", min "); p(float(distrib.min_ns));
      std::fprintf(output, ", max "); p(float(distrib.max_ns));
      std::fprintf(output, "\n");
    }
  }

  void stat_reset_global() {
    GLOBAL_STATS = Stats();
    THREAD_STATS = Stats();
    stat_init_global();
  }

  bool stat_name_matches(const char* name, const std::string& word) {
    return std::string(name).find(word) != std::string::npos;
  }

  void stat_enable(const std::string& word, bool enable) {
    assert(GLOBAL_STATS.initialized);
    assert(!THREAD_STATS.initialized);

    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    if(enable) {
      GLOBAL_STATS.enabled = true;
    }

    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      if(stat_name_matches(STAT_COUNTER_DEFS.at(i).name, word)) {
        GLOBAL_STATS.counters.at(i).enabled = enable;
      }
    }
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      if(stat_name_matches(STAT_DISTRIB_INT_DEFS.at(i).name, word)) {
        GLOBAL_STATS.distrib_ints.at(i).enabled = enable;
      }
    }
    for(uint32_t i = 0; i < _TIMER_END; ++i) {
      if(stat_name_matches(STAT_DISTRIB_TIME_DEFS.at(i).name, word)) {
        GLOBAL_STATS.distrib_times.at(i).enabled = enable;
      }
    }
  }

  StatTimer::StatTimer(StatDistribTime distrib_id) {
    this->distrib_id = distrib_id;
    this->measuring = false;
    this->estimate_overhead = false;
    this->active = true;

    if(THREAD_STATS.distrib_times.at(this->distrib_id).enabled) {
      uint32_t prob = STAT_DISTRIB_TIME_DEFS.at(distrib_id).sample_probability;
      uint32_t rand = THREAD_TIME_SAMPLE_RNG() << (32 - THREAD_TIME_SAMPLE_RNG.word_size);
      if((this->measuring = rand < prob)) {
        this->estimate_overhead = (uint64_t(rand) * 64 / prob) == 0;
        this->time_0 = stat_clock_now_ns();
      }
    }
  }

  StatTimer::~StatTimer() {
    if(this->active) {
      this->stop();
    }
  }

  void StatTimer::stop() {
    assert(this->active);
    auto& distrib = THREAD_STATS.distrib_times.at(this->distrib_id);
    distrib.total_count += 1;
    if(!this->measuring) {
      return;
    }

    int64_t time_1 = stat_clock_now_ns();
    int64_t sample_ns = time_1 - this->time_0;

    if(this->estimate_overhead) {
      auto& o_distrib = THREAD_STATS.timer_overhead_distrib;
      int64_t time_2 = stat_clock_now_ns();
      int64_t o_sample_ns = time_2 - time_1;

      o_distrib.sampled_count += 1;
      o_distrib.sum_ns += o_sample_ns;
    }

    distrib.sampled_count += 1;
    distrib.sum_ns += sample_ns;
    distrib.sum_squares_ns += sample_ns * sample_ns;
    distrib.min_ns = std::min(distrib.min_ns, sample_ns);
    distrib.max_ns = std::max(distrib.max_ns, sample_ns);
    this->active = this->measuring = false;
  }
}
#endif

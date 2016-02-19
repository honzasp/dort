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
    { "bvh isect" },
    { "bvh isect hit" },
    { "bvh isect_p" },
    { "bvh isect_p hit" },
    { "triangle hit" },
    { "triangle hit hit" },
    { "triangle hit_p" },
    { "triangle hit_p hit" },
  };

  const std::vector<StatDistribIntDef> STAT_DISTRIB_INT_DEFS = {
    { "bvh traverse count" },
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
    { "film add_tile", UINT32_MAX / 16 },
    { "sampler start_pixel", UINT32_MAX / 256 },
    { "sampler start_pixel_sample", UINT32_MAX / 256 },
    { "rng float", UINT32_MAX / 1024 },
    { "rng uint32", UINT32_MAX / 1024 },
    { "bvh build", UINT32_MAX },
    { "bvh traverse node", UINT32_MAX / 256 },
    { "bvh intersect prim", UINT32_MAX / 256 },
  };

  StatTimer::StatTimer(StatDistribTime distrib_id) {
    this->distrib_id = distrib_id;
    uint32_t prob = STAT_DISTRIB_TIME_DEFS.at(distrib_id).sample_probability;
    uint32_t rand = THREAD_TIME_SAMPLE_RNG() << (32 - THREAD_TIME_SAMPLE_RNG.word_size);
    if((this->measuring = rand < prob)) {
      this->time_0 = stat_clock_now_ns();
    }
    this->active = true;
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
    THREAD_TIME_SAMPLE_RNG();
    int64_t time_2 = stat_clock_now_ns();
    int64_t overhead_ns = time_2 - time_1;
    int64_t sample_ns = time_1 - this->time_0 - overhead_ns;

    distrib.sampled_count += 1;
    distrib.sum_ns += sample_ns;
    distrib.sum_squares_ns += sample_ns * sample_ns;
    distrib.sum_overhead_ns += overhead_ns;
    distrib.min_ns = std::min(distrib.min_ns, sample_ns);
    distrib.max_ns = std::max(distrib.max_ns, sample_ns);
    this->active = this->measuring = false;
  }

  void stat_init_global() {
    stat_init(GLOBAL_STATS);
    stat_init(THREAD_STATS);
  }

  void stat_init_thread() {
    stat_init(THREAD_STATS);
  }

  void stat_init(Stats& stats) {
    assert(!stats.initialized);
    stats.counters = std::vector<uint64_t>(_COUNTER_END, 0);
    stats.distrib_ints = std::vector<Stats::DistribInt>(_DISTRIB_INT_END);
    stats.distrib_times = std::vector<Stats::DistribTime>(_TIMER_END);
    stats.initialized = true;
  }

  void stat_finish_thread() {
    assert(THREAD_STATS.initialized);
    assert(GLOBAL_STATS.initialized);

    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      GLOBAL_STATS.counters.at(i) += THREAD_STATS.counters.at(i);
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
      glob.sum_overhead_ns += thrd.sum_overhead_ns;
      glob.min_ns = std::min(glob.min_ns, thrd.min_ns);
      glob.max_ns = std::max(glob.max_ns, thrd.max_ns);
    }

    THREAD_STATS.initialized = false;
  }

  void stat_report_global(FILE* output) {
    assert(GLOBAL_STATS.initialized);
    stat_finish_thread();

    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    std::fprintf(output, "# Dort statistics\n");

    std::fprintf(output, "## Counters\n");
    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      std::fprintf(output, "- %-30s  %10" PRIu64 "\n", 
          STAT_COUNTER_DEFS.at(i).name,
          GLOBAL_STATS.counters.at(i));
    }

    std::fprintf(output, "## Integer distributions\n");
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      const auto& distrib = GLOBAL_STATS.distrib_ints.at(i);
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
    for(uint32_t i = 0; i < _TIMER_END; ++i) {
      const auto& distrib = GLOBAL_STATS.distrib_times.at(i);
      std::fprintf(output, "- %-30s  ", STAT_DISTRIB_TIME_DEFS.at(i).name);
      if(distrib.sampled_count == 0) {
        std::fprintf(output, "(no samples)\n");
        continue;
      }

      float average_ns = float(distrib.sum_ns) 
        / float(distrib.sampled_count);
      float average_square_ns = float(distrib.sum_squares_ns) 
        / float(distrib.sampled_count);
      float average_overhead_ns = float(distrib.sum_overhead_ns) 
        / float(distrib.sampled_count);
      float estimate_total_ns = average_ns * float(distrib.total_count);
      float variance = abs(average_square_ns - average_ns * average_ns);
      float stddev_ns = sqrt(variance);
      std::fprintf(output, "avg %g ns, sd %g ns, n %" PRIu64 "/%" PRIu64 ",",
          average_ns, stddev_ns, distrib.sampled_count, distrib.total_count);

      if(abs(estimate_total_ns) < 20e3f) {
        std::fprintf(output, " total ~%3g ns", estimate_total_ns);
      } else if(abs(estimate_total_ns) < 20e6f) {
        std::fprintf(output, " total ~%3g us", estimate_total_ns * 1e-3f);
      } else if(abs(estimate_total_ns) < 20e9f) {
        std::fprintf(output, " total ~%3g ms", estimate_total_ns * 1e-6f);
      } else if(abs(estimate_total_ns) < 2000e12f) {
        std::fprintf(output, " total ~%3g s", estimate_total_ns * 1e-9f);
      } else {
        std::fprintf(output, " total ~%3g min", estimate_total_ns * 1e-9f / 60.f);
      }

      std::fprintf(output, "\n                                  ");
      std::fprintf(output, "min %" PRIi64 " ns, max %" PRIi64 " ns, avg ohead %g ns\n",
          distrib.min_ns, distrib.max_ns, average_overhead_ns);
    }
  }

  int64_t stat_clock_now_ns() {
    struct timespec tp;
    ::clock_gettime(CLOCK_MONOTONIC, &tp);
    return 1000ll*1000ll*1000ll * int64_t(tp.tv_sec) + int64_t(tp.tv_nsec);
  }
}

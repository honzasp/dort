#include <cinttypes>
#include <mutex>
#include "dort/math.hpp"
#include "dort/stats.hpp"

namespace dort {
  Stats GLOBAL_STATS;
  std::mutex GLOBAL_STATS_MUTEX;
  thread_local Stats THREAD_STATS;

  const std::vector<StatCounterDef> stat_counter_defs = {
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
    { "direct get_radiance" },
    { "uniform_sample_all_lights" },
    { "estimate_direct" },
    { "trace_specular" },
  };

  const std::vector<StatDistribIntDef> stat_distrib_int_defs = {
    { "bvh traverse count" },
    { "bsdf number of bxdfs" },
  };

  void stat_init_global() {
    stat_init(GLOBAL_STATS);
  }

  void stat_init_thread() {
    stat_init(THREAD_STATS);
  }

  void stat_init(Stats& stats) {
    assert(!stats.initialized);
    stats.counters = std::vector<uint64_t>(_COUNTER_END, 0);
    stats.distrib_ints = std::vector<Stats::DistribInt>(_DISTRIB_INT_END);
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

    THREAD_STATS.initialized = false;
  }

  void stat_report_global(FILE* output) {
    assert(GLOBAL_STATS.initialized);
    std::unique_lock<std::mutex> global_lock(GLOBAL_STATS_MUTEX);
    std::fprintf(output, "# Dort statistics\n");

    std::fprintf(output, "## Counters\n");
    for(uint32_t i = 0; i < _COUNTER_END; ++i) {
      std::fprintf(output, "- %-30s  %10" PRIu64 "\n", 
          stat_counter_defs.at(i).name,
          GLOBAL_STATS.counters.at(i));
    }

    std::fprintf(output, "## Integer distributions\n");
    for(uint32_t i = 0; i < _DISTRIB_INT_END; ++i) {
      const auto& distrib = GLOBAL_STATS.distrib_ints.at(i);
      std::fprintf(output, "- %-30s  ", stat_distrib_int_defs.at(i).name);
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
  }
}

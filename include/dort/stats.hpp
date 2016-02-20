#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  enum StatCounter: uint32_t {
    COUNTER_SCENE_INTERSECT,
    COUNTER_SCENE_INTERSECT_P,
    COUNTER_BVH_FAST_BOX_INTERSECT_P,
    COUNTER_BVH_FAST_BOX_INTERSECT_P_HIT,
    COUNTER_BVH_INTERSECT,
    COUNTER_BVH_INTERSECT_HIT,
    COUNTER_BVH_INTERSECT_P,
    COUNTER_BVH_INTERSECT_P_HIT,
    COUNTER_TRIANGLE_HIT,
    COUNTER_TRIANGLE_HIT_HIT,
    COUNTER_TRIANGLE_HIT_P,
    COUNTER_TRIANGLE_HIT_P_HIT,
    _COUNTER_END,
  };

  struct StatCounterDef {
    const char* name;
  };

  enum StatDistribInt: uint32_t {
    DISTRIB_INT_BVH_TRAVERSE_COUNT,
    DISTRIB_INT_BVH_BUILD_NODE_PARALLEL_COUNT,
    DISTRIB_INT_BVH_BUILD_NODE_SERIAL_COUNT,
    DISTRIB_INT_BVH_BUILD_SERIAL_COUNT,
    DISTRIB_INT_BVH_BUILD_LINEAR_RESIZE,
    DISTRIB_INT_BSDF_NUM_BXDFS,
    DISTRIB_INT_RENDER_JOBS,
    _DISTRIB_INT_END,
  };

  struct StatDistribIntDef {
    const char* name;
  };

  enum StatDistribTime: uint32_t {
    TIMER_RENDER,
    TIMER_RENDER_TILE,
    TIMER_DIRECT_GET_RADIANCE,
    TIMER_UNIFORM_SAMPLE_ALL_LIGHTS,
    TIMER_ESTIMATE_DIRECT,
    TIMER_TRACE_SPECULAR,
    TIMER_SCENE_INTERSECT,
    TIMER_SCENE_INTERSECT_P,
    TIMER_FILM_ADD_SAMPLE,
    TIMER_FILM_ADD_TILE,
    TIMER_SAMPLER_START_PIXEL,
    TIMER_SAMPLER_START_PIXEL_SAMPLE,
    TIMER_RNG_FLOAT,
    TIMER_RNG_UINT32,
    TIMER_BVH_BUILD,
    TIMER_BVH_COMPUTE_BUILD_INFOS,
    TIMER_BVH_BUILD_NODE_PARALLEL,
    TIMER_BVH_BUILD_NODE_SERIAL,
    TIMER_BVH_SPLIT_MIDDLE,
    TIMER_BVH_SPLIT_MEDIAN,
    TIMER_BVH_SPLIT_SAH,
    TIMER_BVH_TRAVERSE_NODE,
    TIMER_BVH_INTERSECT_PRIM,
    _TIMER_END,
  };

  struct StatDistribTimeDef {
    const char* name;
    uint32_t sample_probability;
  };

  struct Stats {
    struct DistribInt {
      uint64_t count = 0;
      int64_t sum = 0;
      int64_t sum_squares = 0;
      int64_t min = INT64_MAX;
      int64_t max = INT64_MIN;
    };

    struct DistribTime {
      uint64_t total_count = 0;
      uint64_t sampled_count = 0;
      int64_t sum_ns = 0;
      int64_t sum_squares_ns = 0;
      int64_t sum_overhead_ns = 0;
      int64_t min_ns = INT64_MAX;
      int64_t max_ns = INT64_MIN;
    };

    bool initialized = false;
    std::vector<uint64_t> counters;
    std::vector<DistribInt> distrib_ints;
    std::vector<DistribTime> distrib_times;
  };

  extern Stats GLOBAL_STATS;
  extern thread_local Stats THREAD_STATS;

  extern const std::vector<StatCounterDef> STAT_COUNTER_DEFS;
  extern const std::vector<StatDistribIntDef> STAT_DISTRIB_INT_DEFS;
  extern const std::vector<StatDistribTimeDef> STAT_DISTRIB_TIME_DEFS;

  int64_t stat_clock_now_ns();

#ifndef DORT_DISABLE_STAT
  void stat_init_global();
  void stat_init_thread();
  void stat_init(Stats& stats);
  void stat_finish_thread();
  void stat_report_global(FILE* output);

  inline void stat_count(StatCounter id) {
    THREAD_STATS.counters.at(id) += 1;
  }

  class StatTimer {
    StatDistribTime distrib_id;
    int64_t time_0;
    bool active;
    bool measuring;
  public:
    StatTimer(StatDistribTime distrib_id);
    ~StatTimer();
    void stop();
  };

  inline void stat_sample_int(StatDistribInt id, int64_t sample) {
    auto& distrib = THREAD_STATS.distrib_ints.at(id);
    distrib.count += 1;
    distrib.sum += sample;
    distrib.sum_squares += sample * sample;
    distrib.min = std::min(distrib.min, sample);
    distrib.max = std::max(distrib.max, sample);
  }
#else
  inline void stat_init_global() { }
  inline void stat_init_thread() { }
  inline void stat_init(Stats&) { }
  inline void stat_finish_thread() { }
  inline void stat_report_global(FILE*) { }

  inline void stat_count(StatCounter) { }

  class StatTimer {
  public:
    StatTimer(StatDistribTime) { }
    ~StatTimer() { }
    void stop() { }
  };

  inline void stat_sample_int(StatDistribInt, int64_t) { }
#endif
}

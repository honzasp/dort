#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  enum StatCounter: uint32_t {
    COUNTER_SCENE_INTERSECT,
    COUNTER_SCENE_INTERSECT_P,
    COUNTER_BVH_FAST_BOX_INTERSECT_P,
    COUNTER_BVH_FAST_BOX_INTERSECT_P_HIT,
    COUNTER_BVH_PRIM_INTERSECT,
    COUNTER_BVH_PRIM_INTERSECT_HIT,
    COUNTER_BVH_PRIM_INTERSECT_P,
    COUNTER_BVH_PRIM_INTERSECT_P_HIT,
    COUNTER_TRIANGLE_HIT,
    COUNTER_TRIANGLE_HIT_HIT,
    COUNTER_TRIANGLE_HIT_P,
    COUNTER_TRIANGLE_HIT_P_HIT,
    COUNTER_POOL_JOBS,
    COUNTER_POOL_NO_WAITS,
    COUNTER_POOL_WAITS,
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
    DISTRIB_INT_BVH_SPLIT_MIDDLE_JOBS,
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
    TIMER_FILM_ADD_SPLAT,
    TIMER_FILM_ADD_TILE,
    TIMER_RNG_FLOAT,
    TIMER_RNG_UINT32,
    TIMER_BVH_BUILD,
    TIMER_BVH_COMPUTE_BUILD_INFOS,
    TIMER_BVH_BUILD_NODE_PARALLEL,
    TIMER_BVH_BUILD_NODE_SERIAL,
    TIMER_BVH_BUILD_PARTITION_PARALLEL,
    TIMER_BVH_BUILD_PARTITION_SERIAL,
    TIMER_BVH_WRITE_LINEAR_NODE,
    TIMER_BVH_WRITE_LINEAR_NODE_RESIZE,
    TIMER_BVH_SPLIT_MIDDLE,
    TIMER_BVH_SPLIT_MIDDLE_BOUNDS_OUT_PARALLEL,
    TIMER_BVH_SPLIT_MIDDLE_BOUNDS_OUT_SERIAL,
    TIMER_BVH_SPLIT_MIDDLE_BOUNDS_IN_PARALLEL,
    TIMER_BVH_SPLIT_MIDDLE_BOUNDS_IN_SERIAL,
    TIMER_BVH_SPLIT_MEDIAN,
    TIMER_BVH_SPLIT_SAH,
    TIMER_BVH_TRAVERSE_NODE,
    TIMER_BVH_TRAVERSE_ELEM,
    TIMER_POOL_WAIT,
    TIMER_POOL_JOB,
    TIMER_POOL_SCHEDULE,
    _TIMER_END,
  };

  struct StatDistribTimeDef {
    const char* name;
    uint32_t sample_probability;
  };

  struct Stats {
    struct Counter {
      bool enabled = false;
      uint64_t count = 0;
    };

    struct DistribInt {
      bool enabled = false;
      uint64_t count = 0;
      int64_t sum = 0;
      int64_t sum_squares = 0;
      int64_t min = INT64_MAX;
      int64_t max = INT64_MIN;
    };

    struct DistribTime {
      bool enabled = false;
      uint32_t ticks_to_sample = 0;
      uint64_t total_count = 0;
      uint64_t sampled_count = 0;
      int64_t sum_ns = 0;
      int64_t sum_squares_ns = 0;
      int64_t min_ns = INT64_MAX;
      int64_t max_ns = INT64_MIN;
    };

    bool initialized = false;
    bool enabled = false;
    std::vector<Counter> counters;
    std::vector<DistribInt> distrib_ints;
    std::vector<DistribTime> distrib_times;
    DistribTime timer_overhead_distrib;
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
  void stat_reset_global();
  bool stat_name_matches(const char* name, const std::string& word);
  void stat_enable(const std::string& word, bool enable);

  inline void stat_count(StatCounter id) {
    THREAD_STATS.counters.at(id).count += 1;
  }

  class StatTimer {
    StatDistribTime distrib_id;
    int64_t time_0;
    bool active;
    bool measuring;
    bool estimate_overhead;
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
  inline void stat_reset_global() { }
  inline void stat_enable(const std::string&) { }
  inline void stat_disable(const std::string&) { }

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

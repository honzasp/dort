#pragma once
#include <chrono>
#include <cstdint>
#include <cstdio>

template<class T>
void force_eval(T value) {
  volatile T blackhole = value;
  (void)blackhole;
}

template<class F>
int64_t measure_runtime(F fun) {
  using clock = std::chrono::high_resolution_clock;
  auto begin = clock::now();
  fun();
  auto end = clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
}

template<class F>
void benchmark_fun(const char* name, F fun, uint32_t fun_iters) {
  int64_t total_time = 0;
  int64_t total_iters = 0;
  for(uint32_t i = 0; i < 100000 && total_time < 1000*1000*1000; ++i) {
    total_time += measure_runtime([&]() {
      for(uint32_t j = 0; j < 1000; ++j) {
        for(uint32_t k = 0; k < fun_iters; ++k) {
          force_eval(fun(k));
        }
      }
    });
    total_iters += fun_iters * 1000;
  }

  double time_per_iter = double(total_time) / double(total_iters);
  std::printf("%s: %5.2lf ns/iter\n", name, time_per_iter);
}


#include <chrono>
#include "benchmark.hpp"

int main() {
  const uint32_t length = 128;

  benchmark_fun("high_resolution_clock", [&](uint32_t) {
      return std::chrono::high_resolution_clock::now();
    }, length);
  benchmark_fun("system_clock", [&](uint32_t) {
      return std::chrono::system_clock::now();
    }, length);
  benchmark_fun("steady_clock", [&](uint32_t) {
      return std::chrono::steady_clock::now();
    }, length);
  benchmark_fun("clock_gettime CLOCK_REALTIME", [&](uint32_t) {
      struct timespec tp;
      ::clock_gettime(CLOCK_REALTIME, &tp);
      return tp;
    }, length);
  benchmark_fun("clock_gettime CLOCK_MONOTONIC", [&](uint32_t) {
      struct timespec tp;
      ::clock_gettime(CLOCK_MONOTONIC, &tp);
      return tp;
    }, length);
  benchmark_fun("clock_gettime CLOCK_PROCESS_CPUTIME_ID", [&](uint32_t) {
      struct timespec tp;
      ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
      return tp;
    }, length);
  benchmark_fun("clock_gettime CLOCK_THREAD_CPUTIME_ID", [&](uint32_t) {
      struct timespec tp;
      ::clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp);
      return tp;
    }, length);
  return 0;
}

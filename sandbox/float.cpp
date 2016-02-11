#include <cmath>
#include <chrono>
#include <cstdint>
#include <random>

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

int main() {
  const uint32_t length = 64;
  float xs[length];
  float ys[length];

  {
    std::mt19937 rng(42);
    std::exponential_distribution<float> dis_exp(1.f);
    std::uniform_real_distribution<float> dis_unif(-1.f, 1.f);
    for(uint32_t i = 0; i < length; ++i) {
      xs[i] = dis_exp(rng) * dis_unif(rng);
      ys[i] = dis_exp(rng) * dis_unif(rng);
    }
  }

  benchmark_fun("access x", [&](uint32_t i) { return xs[i]; }, length);
  benchmark_fun("access y", [&](uint32_t i) { return ys[i]; }, length);
  benchmark_fun("+       ", [&](uint32_t i) { return xs[i] + ys[i]; }, length);
  benchmark_fun("-       ", [&](uint32_t i) { return xs[i] - ys[i]; }, length);
  benchmark_fun("*       ", [&](uint32_t i) { return xs[i] * ys[i]; }, length);
  benchmark_fun("/       ", [&](uint32_t i) { return xs[i] / ys[i]; }, length);
  benchmark_fun("recip   ", [&](uint32_t i) { return 1.f / xs[i]; }, length);
  benchmark_fun("square  ", [&](uint32_t i) { return xs[i] * xs[i]; }, length);
  benchmark_fun("cube    ", [&](uint32_t i) { return xs[i] * xs[i] * xs[i]; }, length);
  benchmark_fun("sqrt    ", [&](uint32_t i) { return sqrt(xs[i]); }, length);
  benchmark_fun("floor   ", [&](uint32_t i) { return floor(xs[i]); }, length);
  benchmark_fun("ceil    ", [&](uint32_t i) { return ceil(xs[i]); }, length);
  benchmark_fun("exp     ", [&](uint32_t i) { return exp(xs[i]); }, length);
  benchmark_fun("log     ", [&](uint32_t i) { return log(xs[i]); }, length);
  benchmark_fun("pow     ", [&](uint32_t i) { return pow(xs[i], ys[i]); }, length);
  benchmark_fun("sin     ", [&](uint32_t i) { return sin(xs[i]); }, length);
  benchmark_fun("cos     ", [&](uint32_t i) { return cos(xs[i]); }, length);
  benchmark_fun("tan     ", [&](uint32_t i) { return tan(xs[i]); }, length);
  benchmark_fun("asin    ", [&](uint32_t i) { return asin(xs[i]); }, length);
  benchmark_fun("acos    ", [&](uint32_t i) { return acos(xs[i]); }, length);
  benchmark_fun("atan    ", [&](uint32_t i) { return atan(xs[i]); }, length);
  benchmark_fun("atan2   ", [&](uint32_t i) { return atan2(xs[i], ys[i]); }, length);
  return 0;
}

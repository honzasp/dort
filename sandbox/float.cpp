#include <cmath>
#include <cstdint>
#include <random>
#include "benchmark.hpp"

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
  benchmark_fun("sqrt    ", [&](uint32_t i) { return std::sqrt(xs[i]); }, length);
  benchmark_fun("floor   ", [&](uint32_t i) { return std::floor(xs[i]); }, length);
  benchmark_fun("ceil    ", [&](uint32_t i) { return std::ceil(xs[i]); }, length);
  benchmark_fun("exp     ", [&](uint32_t i) { return std::exp(xs[i]); }, length);
  benchmark_fun("log     ", [&](uint32_t i) { return std::log(xs[i]); }, length);
  benchmark_fun("pow     ", [&](uint32_t i) { return std::pow(xs[i], ys[i]); }, length);
  benchmark_fun("sin     ", [&](uint32_t i) { return std::sin(xs[i]); }, length);
  benchmark_fun("cos     ", [&](uint32_t i) { return std::cos(xs[i]); }, length);
  benchmark_fun("tan     ", [&](uint32_t i) { return std::tan(xs[i]); }, length);
  benchmark_fun("asin    ", [&](uint32_t i) { return std::asin(xs[i]); }, length);
  benchmark_fun("acos    ", [&](uint32_t i) { return std::acos(xs[i]); }, length);
  benchmark_fun("atan    ", [&](uint32_t i) { return std::atan(xs[i]); }, length);
  benchmark_fun("atan2   ", [&](uint32_t i) { return std::atan2(xs[i], ys[i]); }, length);
  benchmark_fun("abs     ", [&](uint32_t i) { return std::abs(xs[i]); }, length);
  benchmark_fun("copysign", [&](uint32_t i) { return std::copysign(xs[i], ys[i]); }, length);
  return 0;
}

#include <memory>
#include <random>
#include "benchmark.hpp"

struct Obj4 { uint32_t a; };
struct Obj8 { uint32_t a, b; };
struct Obj16 { uint32_t a, b, c, d; };
struct Obj32 { uint32_t a, b, c, d, e, f, g, h; };
struct Obj64 { uint32_t a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p; };

int main() {
  const uint32_t length = 256;
  std::vector<Obj4> os4(length), os4d(length);
  std::vector<Obj8> os8(length), os8d(length);
  std::vector<Obj16> os16(length), os16d(length);
  std::vector<Obj32> os32(length), os32d(length);
  std::vector<Obj64> os64(length), os64d(length);

  benchmark_fun("obj 4", [&](uint32_t i) { return os4d[i] = os4[i]; }, length);
  benchmark_fun("obj 8", [&](uint32_t i) { return os8d[i] = os8[i]; }, length);
  benchmark_fun("obj 16", [&](uint32_t i) { return os16d[i] = os16[i]; }, length);
  benchmark_fun("obj 32", [&](uint32_t i) { return os32d[i] = os32[i]; }, length);
  benchmark_fun("obj 64", [&](uint32_t i) { return os64d[i] = os64[i]; }, length);
  return 0;
}

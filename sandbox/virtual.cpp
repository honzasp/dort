#include <memory>
#include <chrono>
#include <cstdint>
#include <random>

struct Base {
  virtual int32_t fun(int32_t a, int32_t b) const = 0;
  virtual ~Base() { }
};

struct ChildPlus: public Base {
  virtual int32_t fun(int32_t a, int32_t b) const override final {
    return a + b;
  }
};

struct ChildMinus: public Base {
  virtual int32_t fun(int32_t a, int32_t b) const override final {
    return a - b;
  }
};

struct ChildAnd: public Base {
  virtual int32_t fun(int32_t a, int32_t b) const override final {
    return a & b;
  }
};

struct ChildXor: public Base {
  virtual int32_t fun(int32_t a, int32_t b) const override final {
    return a ^ b;
  }
};

int32_t free_plus(int32_t a, int32_t b) { return a + b; }
int32_t free_minus(int32_t a, int32_t b) { return a - b; }
int32_t free_and(int32_t a, int32_t b) { return a & b; }
int32_t free_xor(int32_t a, int32_t b) { return a ^ b; }


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
  std::vector<Base*> objs_raw;
  std::vector<std::shared_ptr<Base>> objs_shared;
  std::vector<std::unique_ptr<Base>> objs_unique;
  std::vector<int32_t(*)(int32_t, int32_t)> funs;

  {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int32_t> dis(0, 3);
    for(uint32_t i = 0; i < length; ++i) {
      switch(dis(rng)) {
        case 0:
          objs_raw.push_back(new ChildPlus);
          objs_shared.push_back(std::make_shared<ChildPlus>());
          objs_unique.push_back(std::unique_ptr<Base>(new ChildPlus));
          funs.push_back(free_plus);
          break;
        case 1:
          objs_raw.push_back(new ChildMinus);
          objs_shared.push_back(std::make_shared<ChildMinus>());
          objs_unique.push_back(std::unique_ptr<Base>(new ChildMinus));
          funs.push_back(free_minus);
          break;
        case 2:
          objs_raw.push_back(new ChildAnd);
          objs_shared.push_back(std::make_shared<ChildAnd>());
          objs_unique.push_back(std::unique_ptr<Base>(new ChildAnd));
          funs.push_back(free_and);
          break;
        default:
          objs_raw.push_back(new ChildXor);
          objs_shared.push_back(std::make_shared<ChildXor>());
          objs_unique.push_back(std::unique_ptr<Base>(new ChildXor));
          funs.push_back(free_xor);
          break;
      }
    }
  }

  benchmark_fun("raw ptr   ", [&](uint32_t i) { return objs_raw[i]->fun(10, 20); }, length);
  benchmark_fun("shared ptr", [&](uint32_t i) { return objs_shared[i]->fun(10, 20); }, length);
  benchmark_fun("unique ptr", [&](uint32_t i) { return objs_unique[i]->fun(10, 20); }, length);
  benchmark_fun("fun ptr   ", [&](uint32_t i) { return funs[i](10, 20); }, length);
  return 0;
}

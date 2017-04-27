#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  struct CtxG {
    std::shared_ptr<ThreadPool> pool;
    std::vector<std::string> argv;

    CtxG(std::shared_ptr<ThreadPool> pool): pool(pool) { }
  };
}

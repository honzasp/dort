#pragma once
#include "dort/dort.hpp"

namespace dort {
  struct CtxG {
    std::shared_ptr<ThreadPool> pool;

    CtxG(std::shared_ptr<ThreadPool> pool): pool(pool) { }
  };
}

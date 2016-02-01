#pragma once
#include <memory>
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  struct Scene {
    std::unique_ptr<Primitive> primitive;
    std::vector<std::unique_ptr<Light>> lights;
  };
}

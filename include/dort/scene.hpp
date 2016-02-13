#pragma once
#include <memory>
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  struct Scene {
    std::unique_ptr<Primitive> primitive;
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<TriangleMesh>> triangle_meshes;
    std::shared_ptr<Camera> camera;
  };
}

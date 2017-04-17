#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  struct Mesh final {
    std::vector<Point> points;
    std::vector<Vec2> uvs;
    std::vector<Normal> normals;
    std::vector<uint32_t> vertices;
  };

  struct PrimitiveMesh final {
    Mesh mesh;
    std::shared_ptr<Material> material;
  };
}

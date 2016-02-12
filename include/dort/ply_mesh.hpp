#pragma once
#include <cstdio>
#include <vector>
#include "dort/geometry.hpp"

namespace dort {
  struct PlyMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
  };

  bool read_ply(FILE* file, PlyMesh& out_mesh);
  std::shared_ptr<TriangleMesh> ply_to_triangle_mesh(
      const PlyMesh& ply_mesh,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::vector<std::unique_ptr<Primitive>>& out_prims);
}

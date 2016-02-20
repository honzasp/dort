#pragma once
#include <cstdio>
#include <vector>
#include "dort/geometry.hpp"

namespace dort {
  struct PlyMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
  };

  bool read_ply_callbacks(FILE* file, 
      std::function<void(uint32_t point_count, uint32_t face_count)> begin_cb,
      std::function<void(Point pt)> point_cb,
      std::function<void(uint32_t idx_1, uint32_t idx_2, uint32_t idx_3)> face_cb);
  bool read_ply(FILE* file, PlyMesh& out_mesh);

  std::shared_ptr<TriangleMesh> ply_to_triangle_mesh(
      const PlyMesh& ply_mesh,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::vector<std::unique_ptr<Primitive>>& out_prims);
  std::shared_ptr<TriangleMesh> read_ply_to_triangle_mesh(
      FILE* file,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::vector<std::unique_ptr<Primitive>>& out_prims);
}

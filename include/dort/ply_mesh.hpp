#pragma once
#include <cstdio>
#include <vector>
#include "dort/geometry.hpp"

namespace dort {
  struct PlyMesh {
    std::vector<Point> points;
    std::vector<uint32_t> vertices;
    uint32_t triangle_count;
  };

  bool read_ply_callbacks(FILE* file, 
      std::function<void(uint32_t point_count, uint32_t face_count)> begin_cb,
      std::function<void(Point pt)> point_cb,
      std::function<void(uint32_t idx_1, uint32_t idx_2, uint32_t idx_3)> face_cb);

  bool read_ply_to_mesh(FILE* file, const Transform& mesh_to_frame, Mesh& out_mesh,
      std::function<void(uint32_t index)> triangle_callback);
  bool read_ply_to_ply_mesh(FILE* file, PlyMesh& out_mesh);
}

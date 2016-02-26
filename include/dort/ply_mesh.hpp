#pragma once
#include <cstdio>
#include <vector>
#include "dort/geometry.hpp"

namespace dort {
  bool read_ply_callbacks(FILE* file, 
      std::function<void(uint32_t point_count, uint32_t face_count)> begin_cb,
      std::function<void(Point pt)> point_cb,
      std::function<void(uint32_t idx_1, uint32_t idx_2, uint32_t idx_3)> face_cb);

  std::shared_ptr<TriangleMesh> read_ply_to_triangle_mesh(
      FILE* file,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::function<void(const TriangleMesh*, uint32_t index)> triangle_callback);
}

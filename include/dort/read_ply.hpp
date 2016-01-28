#pragma once
#include <cstdio>
#include "dort/triangle_mesh.hpp"

namespace dort {
  bool read_ply(FILE* file, TriangleMesh& out_mesh, std::vector<Triangle>& triangles);
}

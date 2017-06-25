#include <cstdio>
#include <rply.h>
#include <rplyfile.h>
#include "dort/mesh.hpp"
#include "dort/ply_mesh.hpp"
#include "dort/transform.hpp"

namespace dort {
  bool read_ply_callbacks(FILE* file, 
      std::function<void(uint32_t point_count, uint32_t face_count)> begin_cb,
      std::function<void(Point pt)> point_cb,
      std::function<void(uint32_t idx_1, uint32_t idx_2, uint32_t idx_3)> face_cb)
  {
    p_ply ply = ply_open_from_file(file, nullptr, 0, 0);
    if(!ply) {
      return false;
    }
    if(!ply_read_header(ply)) {
      return false;
    }

    struct Ctx {
      decltype(point_cb) point_cb_;
      decltype(face_cb) face_cb_;
      Point point;
      uint32_t face_first_idx;
      uint32_t face_last_idx;
    };
    Ctx ctx = {
      point_cb, face_cb,
      Point(0.f, 0.f, 0.f), -1u, -1u,
    };

    auto point_rply_cb = [](p_ply_argument arg) -> int {
      Ctx* ctx; long coord;
      ply_get_argument_user_data(arg, (void**)&ctx, &coord);

      ctx->point.v[coord] = float(ply_get_argument_value(arg));
      if(coord == 2) {
        ctx->point_cb_(ctx->point);
      }
      return 1;
    };

    auto face_rply_cb = [](p_ply_argument arg) -> int {
      long length, value_index;
      ply_get_argument_property(arg, nullptr, &length, &value_index);
      if(value_index < 0) {
        return 1;
      }

      Ctx* ctx;
      ply_get_argument_user_data(arg, (void**)&ctx, nullptr);
      uint32_t index = ply_get_argument_value(arg);

      if(value_index == 0) {
        ctx->face_first_idx = index;
      } else if(value_index != 1) {
        ctx->face_cb_(ctx->face_first_idx, ctx->face_last_idx, index);
      }

      ctx->face_last_idx = index;
      return 1;
    };

    uint32_t point_count =
    ply_set_read_cb(ply, "vertex", "x", point_rply_cb, &ctx, 0);
    ply_set_read_cb(ply, "vertex", "y", point_rply_cb, &ctx, 1);
    ply_set_read_cb(ply, "vertex", "z", point_rply_cb, &ctx, 2);
    uint32_t face_count =
    ply_set_read_cb(ply, "face", "vertex_indices", face_rply_cb, &ctx, 0);

    begin_cb(point_count, face_count);
    if(!ply_read(ply) || !ply_close(ply)) {
      return false;
    }
    return true;
  }

  bool read_ply_to_mesh(
      FILE* file, const Transform& mesh_to_frame, Mesh& out_mesh,
      std::function<void(uint32_t index)> triangle_callback)
  {
    bool ok = read_ply_callbacks(file,
        [&](uint32_t point_count, uint32_t face_count) {
          out_mesh.points.reserve(point_count);
          out_mesh.vertices.reserve(face_count * 3);
        },
        [&](Point pt) {
          out_mesh.points.push_back(mesh_to_frame.apply(pt));
        },
        [&](uint32_t idx_1, uint32_t idx_2, uint32_t idx_3) {
          out_mesh.vertices.push_back(idx_1);
          out_mesh.vertices.push_back(idx_2);
          out_mesh.vertices.push_back(idx_3);
          triangle_callback(out_mesh.vertices.size() - 3);
        });
    out_mesh.points.shrink_to_fit();
    out_mesh.vertices.shrink_to_fit();

    return ok;
  }

  bool read_ply_to_ply_mesh(FILE* file, PlyMesh& out_mesh) {
    assert(out_mesh.points.empty());
    assert(out_mesh.vertices.empty());
    assert(out_mesh.triangle_count == 0);
    bool ok = read_ply_callbacks(file,
        [&](uint32_t point_count, uint32_t face_count) {
          out_mesh.points.reserve(point_count);
          out_mesh.vertices.reserve(face_count * 3);
        },
        [&](Point pt) {
          out_mesh.points.push_back(pt);
        },
        [&](uint32_t idx_1, uint32_t idx_2, uint32_t idx_3) {
          out_mesh.vertices.push_back(idx_1);
          out_mesh.vertices.push_back(idx_2);
          out_mesh.vertices.push_back(idx_3);
          out_mesh.triangle_count += 1;
        });
    out_mesh.points.shrink_to_fit();
    out_mesh.vertices.shrink_to_fit();
    return ok;
  }
}

#include <cstdio>
#include <rply.h>
#include <rplyfile.h>
#include "dort/read_ply.hpp"

namespace dort {
  bool read_ply(FILE* file, TriangleMesh& out_mesh,
      std::vector<Triangle>& out_triangles)
  {
    p_ply ply = ply_open_from_file(file, nullptr, 0, 0);
    if(!ply) {
      return false;
    }
    if(!ply_read_header(ply)) {
      return false;
    }

    struct Ctx {
      TriangleMesh& out_mesh;
      std::vector<Triangle>& out_triangles;
      Point point;
      uint32_t face_first_idx;
      uint32_t face_last_idx;
      uint32_t triangle_count;
    };
    Ctx ctx = {
      out_mesh, out_triangles, Point(0.f, 0.f, 0.f),
      -1u, -1u, 0,
    };

    auto point_cb = [](p_ply_argument arg) -> int {
      Ctx* ctx; long coord;
      ply_get_argument_user_data(arg, (void**)&ctx, &coord);

      ctx->point.v[coord] = float(ply_get_argument_value(arg));
      if(coord == 2) {
        ctx->out_mesh.points.push_back(ctx->point);
      }
      return 1;
    };

    auto face_cb = [](p_ply_argument arg) -> int {
      long length, value_index;
      ply_get_argument_property(arg, nullptr, &length, &value_index);
      if(value_index < 0) {
        return 1;
      }

      Ctx* ctx;
      ply_get_argument_user_data(arg, (void**)&ctx, nullptr);
      uint32_t index = ply_get_argument_value(arg);
      assert(index < ctx->out_mesh.points.size());

      if(value_index == 0) {
        ctx->face_first_idx = index;
      } else if(value_index != 1) {
        ctx->out_mesh.vertices.push_back(ctx->face_first_idx);
        ctx->out_mesh.vertices.push_back(ctx->face_last_idx);
        ctx->out_mesh.vertices.push_back(index);
        ctx->out_triangles.push_back(Triangle(&ctx->out_mesh, ctx->triangle_count));
        ++ctx->triangle_count;
      }

      ctx->face_last_idx = index;
      return 1;
    };

    uint32_t point_count =
    ply_set_read_cb(ply, "vertex", "x", point_cb, &ctx, 0);
    ply_set_read_cb(ply, "vertex", "y", point_cb, &ctx, 1);
    ply_set_read_cb(ply, "vertex", "z", point_cb, &ctx, 2);
    uint32_t face_count =
    ply_set_read_cb(ply, "face", "vertex_indices", face_cb, &ctx, 0);

    ctx.out_mesh.points.reserve(ctx.out_mesh.points.size() + point_count);
    ctx.out_triangles.reserve(ctx.out_triangles.size() + face_count);

    if(!ply_read(ply) || !ply_close(ply)) {
      return false;
    }
    return true;
  }
}

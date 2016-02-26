#include <cstdio>
#include <rply.h>
#include <rplyfile.h>
#include "dort/ply_mesh.hpp"
#include "dort/triangle_mesh.hpp"

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
      decltype(point_cb) point_cb;
      decltype(face_cb) face_cb;
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
        ctx->point_cb(ctx->point);
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
        ctx->face_cb(ctx->face_first_idx, ctx->face_last_idx, index);
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

  std::shared_ptr<TriangleMesh> read_ply_to_triangle_mesh(
      FILE* file,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::function<void(const TriangleMesh*, uint32_t index)> triangle_callback)
  {
    auto mesh = std::make_shared<TriangleMesh>();
    mesh->material = material;
    mesh->area_light = area_light;

    bool success = read_ply_callbacks(file,
        [&](uint32_t point_count, uint32_t face_count) {
          mesh->points.reserve(point_count);
          mesh->vertices.reserve(face_count * 3);
        },
        [&](Point pt) {
          mesh->points.push_back(mesh_to_frame.apply(pt));
        },
        [&](uint32_t idx_1, uint32_t idx_2, uint32_t idx_3) {
          mesh->vertices.push_back(idx_1);
          mesh->vertices.push_back(idx_2);
          mesh->vertices.push_back(idx_3);
          triangle_callback(mesh.get(), mesh->vertices.size() - 3);
        });
    mesh->vertices.shrink_to_fit();

    if(success) {
      return mesh;
    } else {
      return nullptr;
    }
  }
}

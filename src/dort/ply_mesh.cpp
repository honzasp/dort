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

  bool read_ply(FILE* file, PlyMesh& out_mesh) {
    uint32_t vertex_offset = out_mesh.points.size();
    return read_ply_callbacks(file,
        [&](uint32_t point_count, uint32_t face_count) {
          out_mesh.points.reserve(out_mesh.points.size() + point_count);
          out_mesh.vertices.reserve(out_mesh.vertices.size() + face_count * 3);
        },
        [&](Point pt) {
          out_mesh.points.push_back(pt);
        },
        [&](uint32_t idx_1, uint32_t idx_2, uint32_t idx_3) {
          out_mesh.vertices.push_back(vertex_offset + idx_1);
          out_mesh.vertices.push_back(vertex_offset + idx_2);
          out_mesh.vertices.push_back(vertex_offset + idx_3);
        });
  }

  std::shared_ptr<TriangleMesh> ply_to_triangle_mesh(
      const PlyMesh& ply_mesh,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::vector<std::unique_ptr<Primitive>>& out_prims)
  {
    auto triangle_mesh = std::make_shared<TriangleMesh>();
    triangle_mesh->material = material;
    triangle_mesh->area_light = area_light;
    triangle_mesh->vertices = ply_mesh.vertices;

    triangle_mesh->points.reserve(ply_mesh.points.size());
    for(const Point& point: ply_mesh.points) {
      triangle_mesh->points.push_back(mesh_to_frame.apply(point));
    }

    out_prims.reserve(out_prims.size() + ply_mesh.vertices.size() / 3);
    for(uint32_t i = 0; i * 3 < ply_mesh.vertices.size(); ++i) {
      out_prims.push_back(std::make_unique<TrianglePrimitive>(
            triangle_mesh.get(), i * 3));
    }

    return triangle_mesh;
  }

  std::shared_ptr<TriangleMesh> read_ply_to_triangle_mesh(
      FILE* file,
      std::shared_ptr<Material> material,
      std::shared_ptr<AreaLight> area_light,
      const Transform& mesh_to_frame,
      std::vector<std::unique_ptr<Primitive>>& out_prims)
  {
    auto mesh = std::make_shared<TriangleMesh>();
    mesh->material = material;
    mesh->area_light = area_light;

    bool success = read_ply_callbacks(file,
        [&](uint32_t point_count, uint32_t face_count) {
          mesh->points.reserve(point_count);
          mesh->vertices.reserve(face_count);
        },
        [&](Point pt) {
          mesh->points.push_back(mesh_to_frame.apply(pt));
        },
        [&](uint32_t idx_1, uint32_t idx_2, uint32_t idx_3) {
          mesh->vertices.push_back(idx_1);
          mesh->vertices.push_back(idx_2);
          mesh->vertices.push_back(idx_3);
          out_prims.push_back(std::make_unique<TrianglePrimitive>(
              mesh.get(), mesh->vertices.size() - 3));
        });
    mesh->vertices.shrink_to_fit();

    if(success) {
      return mesh;
    } else {
      return nullptr;
    }
  }
}

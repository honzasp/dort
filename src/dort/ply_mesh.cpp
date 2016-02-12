#include <cstdio>
#include <rply.h>
#include <rplyfile.h>
#include "dort/ply_mesh.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  bool read_ply(FILE* file, PlyMesh& out_mesh) {
    p_ply ply = ply_open_from_file(file, nullptr, 0, 0);
    if(!ply) {
      return false;
    }
    if(!ply_read_header(ply)) {
      return false;
    }

    struct Ctx {
      PlyMesh& out_mesh;
      Point point;
      uint32_t face_first_idx;
      uint32_t face_last_idx;
    };
    Ctx ctx = {
      out_mesh, Point(0.f, 0.f, 0.f), -1u, -1u,
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
    ctx.out_mesh.vertices.reserve(ctx.out_mesh.vertices.size() + face_count * 3);

    if(!ply_read(ply) || !ply_close(ply)) {
      return false;
    }
    return true;
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
            Triangle(triangle_mesh.get(), i)));
    }

    return triangle_mesh;
  }
}

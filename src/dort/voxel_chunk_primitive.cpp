#include "dort/bsdf.hpp"
#include "dort/material.hpp"
#include "dort/voxel_chunk_primitive.hpp"

namespace dort {
  VoxelChunkPrimitive::VoxelChunkPrimitive(const Transform& voxel_to_frame,
      const Vec3i& extent, std::vector<int32_t> voxels,
      std::shared_ptr<Material> material):
    voxel_to_frame(voxel_to_frame), extent(extent),
    voxels(std::move(voxels)), material(material),
    voxel_bounds(Point(), Point(Vec3(this->extent))),
    frame_bounds(voxel_to_frame.apply(this->voxel_bounds))
  {
    assert(this->extent.x >= 0 && this->extent.y >= 0 && this->extent.z >= 0);
  }

  bool VoxelChunkPrimitive::intersect(Ray& ray, Intersection& out_isect) const {
    Ray voxel_ray(this->voxel_to_frame.apply_inv(ray));
    bool hit = false;
    this->traverse_voxels(voxel_ray, 
      [&](Vec3i voxel_pos, Point voxel_p, uint32_t axis, int32_t step) 
    {
      if(this->voxels.at(this->index(voxel_pos)) != 0) {
        Normal voxel_nn;
        voxel_nn.v[axis] = -float(step);
        float u = 0.f, v = 0.f;
        Vector voxel_dpdu = Vector(1.f, 0.f, 0.f);
        Vector voxel_dpdv = Vector(0.f, 1.f, 0.f);

        out_isect.ray_epsilon = 1e-3;
        out_isect.primitive = this;
        auto& diff_geom = out_isect.frame_diff_geom;
        diff_geom.p = this->voxel_to_frame.apply(voxel_p);
        diff_geom.nn = this->voxel_to_frame.apply(voxel_nn);
        diff_geom.u = u;
        diff_geom.v = v;
        diff_geom.dpdu = this->voxel_to_frame.apply(voxel_dpdu);
        diff_geom.dpdv = this->voxel_to_frame.apply(voxel_dpdv);

        out_isect.world_diff_geom = out_isect.frame_diff_geom;
        hit = true;
        return false;
      } else {
        return true;
      }
    });
    return hit;
  }

  bool VoxelChunkPrimitive::intersect_p(const Ray& ray) const {
    Ray voxel_ray(this->voxel_to_frame.apply_inv(ray));
    bool hit = false;
    this->traverse_voxels(voxel_ray, [&](Vec3i, Point, uint32_t, int32_t) {
      hit = true;
      return false;
    });
    return hit;
  }

  Box VoxelChunkPrimitive::bounds() const {
    return this->frame_bounds;
  }

  std::unique_ptr<Bsdf> VoxelChunkPrimitive::get_bsdf(
      const DiffGeom& frame_diff_geom) const
  {
    return this->material->get_bsdf(frame_diff_geom);
  }

  const AreaLight* VoxelChunkPrimitive::get_area_light(const DiffGeom&) const {
    return nullptr;
  }

  template<class F>
  void VoxelChunkPrimitive::traverse_voxels(const Ray& voxel_ray, F callback) const {
    float ray_t;
    if(this->voxel_bounds.contains(voxel_ray.orig)) {
      ray_t = voxel_ray.t_min;
    } else if(!this->voxel_bounds.hit(voxel_ray, ray_t)) {
      return;
    }

    Vec3 rel_start(voxel_ray.point_t(ray_t).v);

    Vec3i voxel_pos;
    Vec3 next_crossing_t;
    Vec3 ray_t_step;
    Vec3i voxel_pos_step;
    for(uint32_t axis = 0; axis < 3; ++axis) {
      voxel_pos[axis] = clamp(floor_int32(rel_start[axis]),
          0, this->extent[axis] - 1);
      if(voxel_ray.dir.v[axis] >= 0.f) {
        ray_t_step[axis] = 1.f / voxel_ray.dir.v[axis];
        float step = floor(rel_start[axis] + 1.f) - rel_start[axis];
        next_crossing_t[axis] = ray_t + step * ray_t_step[axis];
        voxel_pos_step[axis] = 1;
      } else {
        ray_t_step[axis] = -1.f / voxel_ray.dir.v[axis];
        float step = rel_start[axis] - ceil(rel_start[axis] - 1.f);
        next_crossing_t[axis] = ray_t + step * ray_t_step[axis];
        voxel_pos_step[axis] = -1;
      }
    }

    uint32_t last_axis = (-next_crossing_t).max_axis();
    for(;;) {
      bool go_on = callback(voxel_pos, voxel_ray.point_t(ray_t),
          last_axis, voxel_pos_step[last_axis]);
      if(!go_on) {
        break;
      }

      uint32_t axis = (-next_crossing_t).max_axis();
      ray_t = next_crossing_t[axis];
      voxel_pos[axis] += voxel_pos_step[axis];
      next_crossing_t[axis] += ray_t_step[axis];

      if(voxel_pos[axis] < 0 || voxel_pos[axis] >= this->extent[axis]) {
        break;
      }
      last_axis = axis;
    }
  }
}

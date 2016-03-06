#pragma once
#include <vector>
#include "dort/primitive.hpp"
#include "dort/vec_3i.hpp"

namespace dort {
  class VoxelChunkPrimitive: public GeometricPrimitive {
    Transform voxel_to_frame;
    Vec3i extent;
    std::vector<int32_t> voxels;
    std::shared_ptr<Material> material;
    Box voxel_bounds;
    Box frame_bounds;
  public:
    VoxelChunkPrimitive(const Transform& voxel_to_frame, const Vec3i& extent,
        std::vector<int32_t> voxels,
        std::shared_ptr<Material> material);

    virtual bool intersect(Ray& ray, Intersection& out_isect) const override final;
    virtual bool intersect_p(const Ray& ray) const override final;
    virtual Box bounds() const override final;
    virtual std::unique_ptr<Bsdf> get_bsdf(
        const DiffGeom& frame_diff_geom) const override final;
    virtual const AreaLight* get_area_light(
        const DiffGeom& frame_diff_geom) const override final;
  private:
    uint32_t index(const Vec3i& pos) const {
      assert(pos.x >= 0 && pos.x < this->extent.x);
      assert(pos.y >= 0 && pos.y < this->extent.y);
      assert(pos.z >= 0 && pos.z < this->extent.z);
      return pos.x + pos.y * this->extent.x + pos.z * this->extent.x * this->extent.z;
    }

    template<class F>
    void traverse_voxels(const Ray& voxel_ray, F callback) const;
  };
}

#include "dort/texture_maps.hpp"

namespace dort {
  std::shared_ptr<TextureGeom<Vec2>> uv_texture_map() {
    return make_texture<Vec2, const DiffGeom&>([=](const DiffGeom& geom) {
      return Vec2(geom.u, geom.v);
    });
  }

  std::shared_ptr<TextureGeom<Vec2>> xy_texture_map(const Transform& tex_to_world) {
    return make_texture<Vec2, const DiffGeom&>([=](const DiffGeom& geom) {
      Vec3 tex_p = tex_to_world.apply_inv(geom.p).v;
      return Vec2(tex_p.x, tex_p.y);
    });
  }

  std::shared_ptr<TextureGeom<Vec2>> spherical_texture_map(const Transform& tex_to_world) {
    return make_texture<Vec2, const DiffGeom&>([=](const DiffGeom& geom) {
      Vec3 tex_p = tex_to_world.apply_inv(geom.p).v;
      float r = length(tex_p);
      if(r == 0.f) {
        return Vec2(0.f, 0.f);
      }

      float phi = atan(tex_p.y, tex_p.x);
      float theta = acos(tex_p.z / r);
      return Vec2(phi * INV_TWO_PI + 0.5f, theta * INV_PI);
    });
  }

  std::shared_ptr<TextureGeom<Vec2>> cylindrical_texture_map(const Transform& tex_to_world) {
    return make_texture<Vec2, const DiffGeom&>([=](const DiffGeom& geom) {
      Vec3 tex_p = tex_to_world.apply_inv(geom.p).v;
      float r = length(tex_p);
      if(r == 0.f) {
        return Vec2(0.f, 0.f);
      }

      float phi = atan(tex_p.y, tex_p.x);
      float z = tex_p.z / r;
      return Vec2(phi * INV_TWO_PI + 0.5f, z);
    });
  }

  std::shared_ptr<TextureGeom<Vec3>> xyz_texture_map(const Transform& tex_to_world) {
    return make_texture<Vec3, const DiffGeom&>([=](const DiffGeom& geom) {
      return tex_to_world.apply_inv(geom.p).v;
    });
  }
}

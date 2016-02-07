#include "dort/basic_texture_maps.hpp"
#include "dort/texture.hpp"
#include "dort/transform.hpp"

namespace dort {
  std::shared_ptr<TextureMap2d> uv_texture_map_2d() {
    return make_texture_map_2d([](const DiffGeom& diff_geom) {
      return Tex2(diff_geom.u, diff_geom.v);
    });
  }

  std::shared_ptr<TextureMap2d> xy_texture_map_2d(
      const Transform& texture_to_world)
  {
    return make_texture_map_2d([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_world.apply_inv(diff_geom.p).v;
      return Tex2(tex_p.x, tex_p.y);
    });
  }

  std::shared_ptr<TextureMap2d> spherical_texture_map_2d(
      const Transform& texture_to_world) 
  {
    return make_texture_map_2d([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_world.apply_inv(diff_geom.p).v;
      float r = length(tex_p);
      if(r == 0.f) {
        return Tex2(0.f, 0.f);
      }

      float phi = atan(tex_p.y, tex_p.x);
      float theta = acos(tex_p.z / r);
      return Tex2(phi * INV_TWO_PI + 0.5f, theta * INV_PI);
    });
  }

  std::shared_ptr<TextureMap2d> cylindrical_texture_map_2d(
      const Transform& texture_to_world)
  {
    return make_texture_map_2d([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_world.apply_inv(diff_geom.p).v;
      float r = length(tex_p);
      if(r == 0.f) {
        return Tex2(0.f, 0.f);
      }

      float phi = atan(tex_p.y, tex_p.x);
      float z = tex_p.z / r;
      return Tex2(phi * INV_TWO_PI + 0.5f, z);
    });
  }
}

#include "dort/spectrum.hpp"
#include "dort/texture.hpp"
#include "dort/texture_maps.hpp"
#include "dort/transform.hpp"

namespace dort {
  template<class T>
  std::shared_ptr<TextureGeom<T>> uv_texture_map(
      std::shared_ptr<Texture2d<T>> texture)
  {
    return make_texture<T, const DiffGeom&>([=](const DiffGeom& diff_geom) {
      return texture->evaluate(Vec2(diff_geom.u, diff_geom.v));
    });
  }

  template std::shared_ptr<TextureGeom<float>> uv_texture_map(
      std::shared_ptr<Texture2d<float>>);
  template std::shared_ptr<TextureGeom<Vec2>> uv_texture_map(
      std::shared_ptr<Texture2d<Vec2>>);
  template std::shared_ptr<TextureGeom<Vec3>> uv_texture_map(
      std::shared_ptr<Texture2d<Vec3>>);
  template std::shared_ptr<TextureGeom<Spectrum>> uv_texture_map(
      std::shared_ptr<Texture2d<Spectrum>>);

  template<class T>
  std::shared_ptr<TextureGeom<T>> xy_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame)
  {
    return make_texture<T, const DiffGeom&>([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_frame.apply_inv(diff_geom.p.v);
      return texture->evaluate(Vec2(tex_p.x, tex_p.y));
    });
  }

  template<class T>
  std::shared_ptr<TextureGeom<T>> spherical_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame)
  {
    return make_texture<T, const DiffGeom&>([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_frame.apply_inv(diff_geom.p.v);
      float r = length(tex_p);
      if(r == 0.f) {
        return texture->evaluate(Vec2(0.f, 0.f));
      }

      float phi = atan(tex_p.y, tex_p.x);
      float theta = acos(tex_p.z / r);
      return texture->evaluate(phi * INV_TWO_PI + 0.5f, theta * INV_PI);
    });
  }

  template<class T>
  std::shared_ptr<TextureGeom<T>> cylindrical_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame)
  {
    return make_texture<T, const DiffGeom&>([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_frame.apply_inv(diff_geom.p).v;
      float r = length(tex_p);
      if(r == 0.f) {
        return texture->evaluate(Vec2(0.f, 0.f));
      }

      float phi = atan(tex_p.y, tex_p.x);
      float z = tex_p.z / r;
      return Vec2(phi * INV_TWO_PI + 0.5f, z);
    });
  }

  template<class T>
  std::shared_ptr<TextureGeom<T>> xyz_texture_map(
      std::shared_ptr<Texture3d<T>> texture,
      const Transform& texture_to_frame)
  {
    return make_texture<T, const DiffGeom&>([=](const DiffGeom& diff_geom) {
      Vec3 tex_p = texture_to_frame.apply_inv(diff_geom.p).v;
      return texture->evaluate(tex_p);
    });
  }
}

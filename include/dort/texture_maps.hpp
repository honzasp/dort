#pragma once
#include "dort/dort.hpp"
#include "dort/texture.hpp"
#include "dort/texture_maps.hpp"
#include "dort/transform.hpp"

namespace dort {
  std::shared_ptr<TextureGeom<Vec2>> uv_texture_map();
  std::shared_ptr<TextureGeom<Vec2>> xy_texture_map(const Transform& tex_to_world);
  std::shared_ptr<TextureGeom<Vec2>> cylindrical_texture_map(const Transform& tex_to_world);
  std::shared_ptr<TextureGeom<Vec2>> spherical_texture_map(const Transform& tex_to_world);
  std::shared_ptr<TextureGeom<Vec3>> xyz_texture_map(const Transform& tex_to_world);
}

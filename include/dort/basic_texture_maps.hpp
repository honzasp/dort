#pragma once
#include "dort/dort.hpp"

namespace dort {
  std::shared_ptr<TextureMap2d> uv_texture_map_2d();
  std::shared_ptr<TextureMap2d> xy_texture_map_2d(
      const Transform& texture_to_world);
  std::shared_ptr<TextureMap2d> spherical_texture_map_2d(
      const Transform& texture_to_world);
  std::shared_ptr<TextureMap2d> cylindrical_texture_map_2d(
      const Transform& texture_to_world);
}

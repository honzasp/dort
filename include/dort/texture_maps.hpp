#pragma once
#include "dort/dort.hpp"

namespace dort {
  template<class T>
  std::shared_ptr<TextureGeom<T>> uv_texture_map(
      std::shared_ptr<Texture2d<T>> texture);

  template<class T>
  std::shared_ptr<TextureGeom<T>> xy_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame);

  template<class T>
  std::shared_ptr<TextureGeom<T>> spherical_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame);

  template<class T>
  std::shared_ptr<TextureGeom<T>> cylindrical_texture_map(
      std::shared_ptr<Texture2d<T>> texture,
      const Transform& texture_to_frame);

  template<class T>
  std::shared_ptr<TextureGeom<T>> xyz_texture_map(
      std::shared_ptr<Texture3d<T>> texture,
      const Transform& texture_to_frame);
}

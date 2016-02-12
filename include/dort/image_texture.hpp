#pragma once
#include "dort/texture.hpp"

namespace dort {
  class ImageTexture final: public Texture<Spectrum> {
    std::shared_ptr<TextureMap2d> texture_map;
    std::shared_ptr<Image<PixelRgb8>> image;
  public:
    ImageTexture(std::shared_ptr<TextureMap2d> texture_map,
        std::shared_ptr<Image<PixelRgb8>> image):
      texture_map(texture_map), image(image)
    { }

    virtual Spectrum evaluate(const DiffGeom& diff_geom) const override final;
  };

  inline
  std::shared_ptr<Texture<Spectrum>> image_texture(
    std::shared_ptr<TextureMap2d> texture_map,
    std::shared_ptr<Image<PixelRgb8>> image)
  {
    return std::make_shared<ImageTexture>(texture_map, image);
  }
}

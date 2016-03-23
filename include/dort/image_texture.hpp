#pragma once
#include "dort/texture.hpp"

namespace dort {
  class ImageTexture final: public Texture<Spectrum, Vec2> {
    std::shared_ptr<Image<PixelRgb8>> image;
  public:
    ImageTexture(std::shared_ptr<Image<PixelRgb8>> image): image(image) { }
    virtual Spectrum evaluate(Vec2 v) const override final;
  };
}

#pragma once
#include "dort/dort.hpp"

namespace dort {
  template<class T>
  class Texture {
  public:
    virtual T evaluate(const DiffGeom& diff_geom) const = 0;
  };

  template<class T, class F>
  class FunctionTexture: public Texture<T> {
    F function;
  public:
    FunctionTexture(F function): function(std::move(function)) { }
    virtual T evaluate(const DiffGeom& diff_geom) const override final {
      return this->function(diff_geom);
    }
  };

  template<class T, class F>
  std::shared_ptr<Texture<T>> make_texture(F function) {
    return std::make_shared<FunctionTexture<T, F>>(std::move(function));
  }

  struct Tex2 {
    float s;
    float t;

    Tex2(float s, float t): s(s), t(t) { }
  };

  class TextureMap2d {
  public:
    virtual Tex2 map(const DiffGeom& diff_geom) const = 0;
  };

  template<class F>
  class FunctionTextureMap2d: public TextureMap2d {
    F function;
  public:
    FunctionTextureMap2d(F function): function(std::move(function)) { }
    virtual Tex2 map(const DiffGeom& diff_geom) const override final {
      return this->function(diff_geom);
    }
  };

  template<class F>
  std::shared_ptr<TextureMap2d> make_texture_map_2d(F function) {
    return std::make_shared<FunctionTextureMap2d<F>>(std::move(function));
  }
}

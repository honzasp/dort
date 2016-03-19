#pragma once
#include "dort/dort.hpp"
#include "dort/vec_2.hpp"
#include "dort/vec_3.hpp"

namespace dort {
  template<class T>
  class Texture {
  public:
    virtual T evaluate(const DiffGeom& diff_geom) const = 0;
  };

  template<class T, class F>
  class FunctionTexture final: public Texture<T> {
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

  class TextureMap2d {
  public:
    virtual Vec2 map(const DiffGeom& diff_geom) const = 0;
  };

  template<class F>
  class FunctionTextureMap2d final: public TextureMap2d {
    F function;
  public:
    FunctionTextureMap2d(F function): function(std::move(function)) { }
    virtual Vec2 map(const DiffGeom& diff_geom) const override final {
      return this->function(diff_geom);
    }
  };

  template<class F>
  std::shared_ptr<TextureMap2d> make_texture_map_2d(F function) {
    return std::make_shared<FunctionTextureMap2d<F>>(std::move(function));
  }

  class TextureMap3d {
  public:
    virtual Vec3 map(const DiffGeom& diff_geom) const = 0;
  };

  template<class F>
  class FunctionTextureMap3d final: public TextureMap3d {
    F function;
  public:
    FunctionTextureMap3d(F function): function(std::move(function)) { }
    virtual Vec3 map(const DiffGeom& diff_geom) const override final {
      return this->function(diff_geom);
    }
  };

  template<class F>
  std::shared_ptr<TextureMap3d> make_texture_map_3d(F function) {
    return std::make_shared<FunctionTextureMap2d<F>>(std::move(function));
  }
}

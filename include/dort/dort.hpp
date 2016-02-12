#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace dort {
  class AreaLight;
  class Bsdf;
  class FramePrimitive;
  class GeometricPrimitive;
  class Light;
  class Material;
  class Primitive;
  class Renderer;
  class Rng;
  class Shape;
  class ShapePrimitive;
  class TextureMap2d;
  class Transform;

  struct Bxdf;
  struct DiffGeom;
  struct Intersection;
  struct PixelRgb8;
  struct Point;
  struct Ray;
  struct RgbSpectrum;
  struct Scene;
  struct ShadowTest;
  struct Tex2;
  struct Vector;

  template<class T> class Texture;
  template<class Pixel> struct Image;

  using Spectrum = RgbSpectrum;
}

#ifndef __cpp_lib_make_unique
namespace std {
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif

#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace dort {
  class AreaLight;
  class Bsdf;
  class Camera;
  class Filter;
  class FramePrimitive;
  class GeometricPrimitive;
  class Light;
  class Material;
  class Primitive;
  class Renderer;
  class Rng;
  class Sampler;
  class Shape;
  class ShapePrimitive;
  class TextureMap2d;
  class ThreadPool;
  class Transform;

  struct Bxdf;
  struct CtxG;
  struct DiffGeom;
  struct Film;
  struct Intersection;
  struct Mesh;
  struct PixelRgb8;
  struct PlyMesh;
  struct Point;
  struct PrimitiveMesh;
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

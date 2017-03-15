#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace dort {
  class AnyTexture;
  class Bsdf;
  class Camera;
  class DiscreteDistrib1d;
  class Filter;
  class FramePrimitive;
  class GeometricPrimitive;
  class Grid;
  class Light;
  class Material;
  class Primitive;
  class Progress;
  class Renderer;
  class Rng;
  class SampledFilter;
  class Sampler;
  class SampleRenderer;
  class Shape;
  class ShapePrimitive;
  class ThreadPool;
  class Transform;

  struct AtomicRgbSpectrum;
  struct Box;
  struct Boxi;
  struct Bxdf;
  struct CtxG;
  struct DiffGeom;
  struct Film;
  struct Intersection;
  struct LightingGeom;
  struct Mesh;
  struct Normal;
  struct PixelRgb8;
  struct PixelRgbFloat;
  struct PlyMesh;
  struct Point;
  struct PrimitiveMesh;
  struct Ray;
  struct Rect;
  struct Recti;
  struct RgbSpectrum;
  struct SampleIdx;
  struct Scene;
  struct ShadowTest;
  struct Tex2;
  struct Vec2;
  struct Vec2i;
  struct Vec3;
  struct Vec3i;
  struct Vector;
  struct VoxelLump;

  template<class Out, class In> class Texture;
  template<class Out> class TextureMap;
  template<class Pixel> struct Image;

  using Spectrum = RgbSpectrum;
  using AtomicSpectrum = AtomicRgbSpectrum;

  template<class T>
  using TextureGeom = Texture<T, const DiffGeom&>;
  template<class T>
  using Texture1d = Texture<T, float>;
  template<class T>
  using Texture2d = Texture<T, Vec2>;
  template<class T>
  using Texture3d = Texture<T, Vec3>;
}

#ifndef __cpp_lib_make_unique
namespace std {
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif

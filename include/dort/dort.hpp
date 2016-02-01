#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>

namespace dort {
  struct Point;
  struct Vector;
  struct Ray;

  struct RgbSpectrum;

  struct Scene;
  struct Intersection;
  class Primitive;
  class Shape;
  struct ShadowTest;
  class Light;
  class Renderer;
}

#ifndef __cpp_lib_make_unique
namespace std {
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
#endif

#pragma once
#include "dort/math.hpp"
#include "dort/vec.hpp"

namespace dort {
  struct Point {
    Vec3 v;

    Point(): v() {}
    Point(float x, float y, float z): v(x, y, z) {}
    explicit Point(const Vec3& v): v(v) {}
  };

  struct Vector {
    Vec3 v;

    Vector(): v() {}
    Vector(float x, float y, float z): v(x, y, z) {}
    explicit Vector(const Vec3& v): v(v) {}
  };

  inline Point operator+(const Point& pt, const Vector& vec) {
    return Point(pt.v + vec.v);
  }
  inline Vector operator-(const Point& pt1, const Point& pt2) {
    return Vector(pt1.v - pt2.v);
  }
  inline Vector operator*(const Vector& vec, float a) {
    return Vector(vec.v * a);
  }
  inline Vector operator*(float a, const Vector& vec) {
    return Vector(a * vec.v);
  }

  inline Vector normalize(const Vector& vec) {
    return Vector(normalize(vec.v));
  }

  inline float dot(const Vector& vec1, const Vector& vec2) {
    return dot(vec1.v, vec2.v);
  }

  template<class T>
  float abs_dot(const T& v1, const T& v2) {
    return abs(dot(v1, v2));
  }

  inline bool is_finite(Vector vec) {
    return is_finite(vec.v);
  }
  inline bool is_finite(Point pt) {
    return is_finite(pt.v);
  }

  struct Ray {
    Point orig;
    Vector dir;
    float t_min;
    float t_max;

    Ray(Point orig, Vector dir, float t_min = 0.f, float t_max = INFINITY):
      orig(orig), dir(dir), t_min(t_min), t_max(t_max)
    { }

    Point operator()(float t) const {
      return this->orig + this->dir * t;
    }
  };
}

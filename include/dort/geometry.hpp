#pragma once
#include "dort/math.hpp"
#include "dort/vec.hpp"

namespace dort {
  struct Point;
  struct Vector;
  struct Normal;

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

  struct Normal {
    Vec3 v;

    Normal(): v() {}
    Normal(float x, float y, float z): v(x, y, z) {}
    explicit Normal(const Vec3& v): v(v) {}
    explicit Normal(const Vector& vec): v(vec.v) {}
  };

  inline Point operator+(const Point& pt, const Vector& vec) {
    return Point(pt.v + vec.v);
  }
  inline Point operator+(const Point& pt1, const Point& pt2) {
    return Point(pt1.v + pt2.v);
  }
  inline Vector operator+(const Vector& vec1, const Vector& vec2) {
    return Vector(vec1.v + vec2.v);
  }
  inline Point operator-(const Point& pt, const Vector& vec) {
    return Point(pt.v - vec.v);
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
  inline Point operator*(const Point& pt, float a) {
    return Point(pt.v * a);
  }
  inline Point operator*(float a, const Point& pt) {
    return Point(a * pt.v);
  }

  inline Vector normalize(const Vector& vec) {
    return Vector(normalize(vec.v));
  }
  inline float dot(const Vector& vec1, const Vector& vec2) {
    return dot(vec1.v, vec2.v);
  }
  inline Vector cross(const Vector& vec1, const Vector& vec2) {
    return Vector(cross(vec1.v, vec2.v));
  }

  inline Normal operator+(const Normal& n1, const Normal& n2) {
    return Normal(n1.v + n2.v);
  }

  inline Normal normalize(const Normal& norm) {
    return Normal(normalize(norm.v));
  }
  inline float dot(const Normal& n1, const Normal& n2) {
    return dot(n1.v, n2.v);
  }

  template<class T>
  float abs_dot(const T& v1, const T& v2) {
    return abs(dot(v1, v2));
  }

  inline bool is_finite(const Vector& vec) {
    return is_finite(vec.v);
  }
  inline bool is_finite(const Point& pt) {
    return is_finite(pt.v);
  }
  inline bool is_finite(const Normal& norm) {
    return is_finite(norm.v);
  }

  struct Ray {
    Point orig;
    Vector dir;
    float t_min;
    float t_max;

    Ray(Point orig, Vector dir, float t_min = -INFINITY, float t_max = INFINITY):
      orig(orig), dir(dir), t_min(t_min), t_max(t_max)
    { }

    Point operator()(float t) const {
      return this->orig + this->dir * t;
    }
  };

  struct Box {
    Point p_min;
    Point p_max;

    Box(): 
      p_min(INFINITY, INFINITY, INFINITY),
      p_max(-INFINITY, INFINITY, INFINITY) { }
    Box(const Point& p_min, const Point& p_max):
      p_min(p_min), p_max(p_max) { }
  };

  Box union_box(const Box& b1, const Box& b2);
  Box union_box(const Box& box, const Point& pt);
}

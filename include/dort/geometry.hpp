#pragma once
#include "dort/math.hpp"
#include "dort/vec_3.hpp"

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
  inline Point operator+(const Vector& vec, const Point& pt) {
    return Point(vec.v + pt.v);
  }
  inline Point operator+(const Point& pt1, const Point& pt2) {
    return Point(pt1.v + pt2.v);
  }
  inline Vector operator+(const Vector& vec1, const Vector& vec2) {
    return Vector(vec1.v + vec2.v);
  }
  inline Vector operator-(const Vector& vec1, const Vector& vec2) {
    return Vector(vec1.v - vec2.v);
  }
  inline Normal operator+(const Normal& n1, const Normal& n2) {
    return Normal(n1.v + n2.v);
  }
  inline Point operator-(const Point& pt, const Vector& vec) {
    return Point(pt.v - vec.v);
  }
  inline Vector operator-(const Point& pt1, const Point& pt2) {
    return Vector(pt1.v - pt2.v);
  }
  inline Vector operator-(const Vector& vec) {
    return Vector(-vec.v);
  }
  inline Vector operator*(const Vector& vec, float a) {
    return Vector(vec.v * a);
  }
  inline Vector operator*(float a, const Vector& vec) {
    return Vector(a * vec.v);
  }
  inline Normal operator*(const Normal& norm, float a) {
    return Normal(norm.v * a);
  }
  inline Vector operator/(const Vector& vec, float a) {
    return Vector(vec.v / a);
  }
  inline Normal operator/(const Normal& norm, float a) {
    return Normal(norm.v / a);
  }
  inline Point operator*(const Point& pt, float a) {
    return Point(pt.v * a);
  }
  inline Point operator*(float a, const Point& pt) {
    return Point(a * pt.v);
  }

  inline bool operator==(const Vector& vec1, const Vector& vec2) {
    return vec1.v == vec2.v;
  }
  inline bool operator==(const Point& pt1, const Point& pt2) {
    return pt1.v == pt2.v;
  }
  inline bool operator==(const Normal& norm1, const Normal& norm2) {
    return norm1.v == norm2.v;
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

  inline Normal normalize(const Normal& norm) {
    return Normal(normalize(norm.v));
  }
  inline float dot(const Normal& n1, const Normal& n2) {
    return dot(n1.v, n2.v);
  }
  inline float dot(const Vector& vec, const Normal& norm) {
    return dot(vec.v, norm.v);
  }

  inline float abs_dot(const Vector& vec1, const Vector& vec2) {
    return abs(dot(vec1.v, vec2.v));
  }
  inline float abs_dot(const Vector& vec, const Normal& norm) {
    return abs(dot(vec.v, norm.v));
  }
  inline float abs_dot(const Normal& norm1, const Normal& norm2) {
    return abs(dot(norm1.v, norm2.v));
  }

  inline float length_squared(const Vector& vec) {
    return length_squared(vec.v);
  }
  inline float length(const Vector& vec) {
    return length(vec.v);
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

  inline Vector abs(const Vector& vec) {
    return Vector(abs(vec.v));
  }

  void coordinate_system(const Vector& vec0, Vector& vec1, Vector& vec2);

  struct Ray {
    Point orig;
    Vector dir;
    float t_min;
    float t_max;

    Ray(const Point& orig, const Vector& dir,
        float t_min = -INFINITY, float t_max = INFINITY):
      orig(orig), dir(dir), t_min(t_min), t_max(t_max)
    { }

    Point point_t(float t) const {
      return this->orig + this->dir * t;
    }
  };

  struct Box {
    union {
      struct {
        Point p_min;
        Point p_max;
      };
      Point p[2];
    };

    Box(): 
      p_min(INFINITY, INFINITY, INFINITY),
      p_max(-INFINITY, -INFINITY, -INFINITY) { }
    Box(const Box& box):
      p_min(box.p_min), p_max(box.p_max) { }
    Box(const Point& p_min, const Point& p_max):
      p_min(p_min), p_max(p_max) { }
    Box& operator=(const Box& box) {
      this->p_min = box.p_min;
      this->p_max = box.p_max;
      return *this;
    }

    const Point& operator[](uint32_t i) const {
      assert(i <= 1);
      return this->p[i];
    }

    float area() const {
      Vec3 r = (this->p_max - this->p_min).v;
      return 2.f * (r.y * r.z + r.x * r.z + r.y * r.z);
    }

    Point centroid() const {
      return (this->p_max + this->p_min) * 0.5f;
    }

    uint8_t max_axis() const {
      return (this->p_max - this->p_min).v.max_axis();
    }

    bool contains(const Point& pt) const {
      return 
        this->p_min.v.x <= pt.v.x && pt.v.x <= this->p_max.v.x &&
        this->p_min.v.y <= pt.v.y && pt.v.y <= this->p_max.v.y &&
        this->p_min.v.z <= pt.v.z && pt.v.z <= this->p_max.v.z;
    }

    bool hit(const Ray& ray, float& out_t) const;
    bool hit_p(const Ray& ray) const;
    bool fast_hit_p(const Ray& ray,
        const Vector& inv_dir, bool dir_is_neg[3]) const;
  };

  Box union_box(const Box& b1, const Box& b2);
  Box union_box(const Box& box, const Point& pt);

  inline bool is_finite(const Box& box) {
    return is_finite(box.p_min) && is_finite(box.p_max);
  }
}

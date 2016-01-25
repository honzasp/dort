#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>

#define DORT_ASSERT_ACTIVE(expr) assert(expr)
#define DORT_ASSERT_INACTIVE(expr) ((void)0)

#define assert_0 DORT_ASSERT_ACTIVE
#define assert_1 DORT_ASSERT_ACTIVE
#define assert_2 DORT_ASSERT_ACTIVE

namespace dort {
  struct Point;
  struct Vector;
  struct Ray;

  struct Intersection;
  class Shape;
}

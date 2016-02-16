#pragma once
#include "dort/vec_2.hpp"

namespace dort {
  class Filter {
  public:
    Vec2 radius;
    Vec2 inv_radius;

    Filter(Vec2 radius): radius(radius), inv_radius(1.f / radius) { }
    virtual ~Filter() { }
    virtual float evaluate(Vec2 p) const = 0;
  };

  class BoxFilter: public Filter {
  public:
    BoxFilter(Vec2 radius): Filter(radius) { }
    virtual float evaluate(Vec2 p) const override final;
  };

  class TriangleFilter: public Filter {
  public:
    TriangleFilter(Vec2 radius): Filter(radius) { }
    virtual float evaluate(Vec2 p) const override final;
  };

  class GaussianFilter: public Filter {
    float alpha;
  public:
    GaussianFilter(Vec2 radius, float alpha):
      Filter(radius), alpha(alpha) { }
    virtual float evaluate(Vec2 p) const override final;
  };

  class MitchellFilter: public Filter {
    float b, c;
  public:
    MitchellFilter(Vec2 radius, float b, float c):
      Filter(radius), b(b), c(c) { }
    virtual float evaluate(Vec2 p) const override final;
  private:
    float evaluate_cubic(float x) const;
  };

  class LanczosSincFilter: public Filter {
    float inv_tau;
  public:
    LanczosSincFilter(Vec2 radius, float tau):
      Filter(radius), inv_tau(1.f / tau) { }
    virtual float evaluate(Vec2 p) const override final;
  };
}

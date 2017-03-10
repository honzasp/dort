#include "dort/filter.hpp"

namespace dort {
  float BoxFilter::evaluate(Vec2) const {
    return 1.f;
  }

  float TriangleFilter::evaluate(Vec2 p) const {
    float f_x = 1.f - abs(p.x * this->inv_radius.x);
    float f_y = 1.f - abs(p.y * this->inv_radius.y);
    return max(0.f, f_x * f_y);
  }

  float GaussianFilter::evaluate(Vec2 p) const {
    float a = this->alpha;
    float rx = this->radius.x;
    float ry = this->radius.y;
    float val_x = exp(-a * square(p.x)) - exp(-a * square(rx));
    float val_y = exp(-a * square(p.y)) - exp(-a * square(ry));
    return max(0.f, val_x) * max(0.f, val_y);
  }

  float MitchellFilter::evaluate(Vec2 p) const {
    return this->evaluate_cubic(2.f * abs(p.x) * this->inv_radius.x) *
      this->evaluate_cubic(2.f * abs(p.y) * this->inv_radius.y);
  }
  
  float MitchellFilter::evaluate_cubic(float x) const {
    if(x < 1.f) {
      return (12.f - 9.f * this->b - 6.f * this->c) * cube(x) +
        (-18.f + 12.f * this->b + 6.f * this->c) * square(x) +
        (6.f - 2.f * this->b);
    } else {
      return (-this->b - 6.f * this->c) * cube(x) +
        (6.f * this->b + 30.f * this->c) * square(x) +
        (-12.f * this->b - 48.f * this->c) * x +
        (8.f * this->b + 24.f * this->c);
    }
  }

  float LanczosSincFilter::evaluate(Vec2 p) const {
    return sinc(p.x) * sinc(p.y) *
      sinc(p.x * this->inv_tau) * sinc(p.y * this->inv_tau);
  }
}

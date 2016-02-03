#include "dort/math.hpp"

namespace dort {
  bool solve_quadratic(float A, float B, float C, float& out_x1, float& out_x2) {
    float discrim = B * B - 4.f * A * C;
    if(discrim <= 0.f) {
      return false;
    }
    float sqrt_discrim = sqrt(discrim);

    float q;
    if(B < 0.f) {
      q = -0.5f * (B - sqrt_discrim);
    } else {
      q = -0.5f * (B + sqrt_discrim);
    }

    out_x1 = q / A;
    out_x2 = C / q;
    return true;
  }
}

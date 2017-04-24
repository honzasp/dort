#include "dort/math.hpp"

namespace dort {
  float normal_cdf_inverse(float p) {
    // https://www.johndcook.com/blog/cpp_phi_inverse/
    auto rational_approx = [](float t) -> float {
      static const float c[] = {2.515517, 0.802853, 0.010328};
      static const float d[] = {1.432788, 0.189269, 0.001308};
      return t - ((c[2]*t + c[1])*t + c[0]) / 
                (((d[2]*t + d[1])*t + d[0])*t + 1.0);
    };

    if(p <= 0.f || p >= 1.f) { return SIGNALING_NAN; }
    if (p < 0.5f) {
      return -rational_approx(sqrt(-2.0f*log(p)) );
    } else {
      return rational_approx(sqrt(-2.0f*log(1.f - p)) );
    }
  }

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

#include "dort/math.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  Vector uniform_hemisphere_sample(float u1, float u2) {
    float z = sqrt(1.f - u1 * u1);
    float theta = TWO_PI * u2;
    float x = cos(theta) * u1;
    float y = sin(theta) * u1;
    return Vector(x, y, z);
  }

  float uniform_hemisphere_pdf(const Vector& w) {
    return (w.v.z >= 0.f) ? INV_TWO_PI : 0.f;
  }

  Vector uniform_disk_sample(float u1, float u2) {
    float r = sqrt(u1);
    float theta = TWO_PI * u2;
    return Vector(cos(theta) * r, sin(theta) * r, 0.f);
  }

  float uniform_disk_pdf() {
    return INV_PI;
  }

  Vector cosine_hemisphere_sample(float u1, float u2) {
    Vector w = uniform_disk_sample(u1, u2);
    w.v.z = sqrt(1.f - w.v.x * w.v.x - w.v.y * w.v.y);
    return w;
  }

  float cosine_hemisphere_pdf(const Vector& w) {
    return w.v.z * INV_PI;
  }

  float mis_power_heuristic(int32_t num_a, float pdf_a,
      int32_t num_b, float pdf_b)
  {
    float a = num_a * pdf_a;
    float b = num_b * pdf_b;
    return (a*a) / (a*a + b*b);
  }
}

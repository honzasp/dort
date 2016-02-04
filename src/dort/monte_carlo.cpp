#include "dort/math.hpp"
#include "dort/monte_carlo.hpp"

namespace dort {
  Vector uniform_hemisphere_sample(float u1, float u2) {
    float z = u1;
    float r = sqrt(max(0.f, 1.f - z * z));
    float phi = TWO_PI * u2;
    float x = cos(phi) * r;
    float y = sin(phi) * r;
    return Vector(x, y, z);
  }

  float uniform_hemisphere_pdf(const Vector& w) {
    return (w.v.z >= 0.f) ? INV_TWO_PI : 0.f;
  }

  Vector uniform_sphere_sample(float u1, float u2) {
    float z = 1.f - 2.f * u1;
    float r = sqrt(max(0.f, 1.f - z * z));
    float phi = TWO_PI * u2;
    float x = cos(phi) * r;
    float y = sin(phi) * r;
    return Vector(x, y, z);
  }

  float uniform_sphere_pdf() {
    return INV_FOUR_PI;
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
    w.v.z = sqrt(max(0.f, 1.f - w.v.x * w.v.x - w.v.y * w.v.y));
    return w;
  }

  float cosine_hemisphere_pdf(const Vector& w) {
    return w.v.z * INV_PI;
  }

  Vector uniform_cone_sample(float cos_theta_max, float u1, float u2) {
    float cos_theta = lerp(u1, 1.0f, cos_theta_max);
    float sin_theta = sqrt(max(0.f, 1.f - cos_theta * cos_theta));
    float phi = TWO_PI * u2;
    float x = cos(phi) * sin_theta;
    float y = sin(phi) * sin_theta;
    float z = cos_theta;
    return Vector(x, y, z);
  }

  float uniform_cone_pdf(float cos_theta_max) {
    return 1.f / (TWO_PI * (1.f - cos_theta_max));
  }

  float mis_power_heuristic(int32_t num_a, float pdf_a,
      int32_t num_b, float pdf_b)
  {
    float a = num_a * pdf_a;
    float b = num_b * pdf_b;
    return (a*a) / (a*a + b*b);
  }
}

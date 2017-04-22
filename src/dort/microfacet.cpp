#include "dort/geometry.hpp"
#include "dort/math.hpp"
#include "dort/microfacet.hpp"
#include "dort/vec_2.hpp"

namespace dort {
  MicrofacetDistrib::MicrofacetDistrib(MicrofacetType type, float roughness) {
    switch(this->type = type) {
      case MicrofacetType::Beckmann:
      case MicrofacetType::GGX:
        this->alpha = roughness;
        break;
      case MicrofacetType::Phong:
        this->alpha = 2.f / square(roughness) - 2.f;
        break;
    }
  }

  float MicrofacetDistrib::d(const Vector& m) const {
    float cos_square = square(m.v.z);
    float sin_square = 1.f - cos_square;
    float tan_square = sin_square / cos_square;

    switch(this->type) {
      case MicrofacetType::Beckmann:
        return INV_PI * exp(-tan_square / square(this->alpha))
          / (square(this->alpha) * square(cos_square));
      case MicrofacetType::Phong:
        return (this->alpha + 2.f) * INV_TWO_PI 
          * pow(cos_square, 0.5f * this->alpha);
      case MicrofacetType::GGX:
        return INV_PI * square(this->alpha)
          / (square(cos_square * (square(this->alpha) + tan_square)));
    };
    return 0.f;
  }

  float MicrofacetDistrib::g1(const Vector& v, const Vector& m) const {
    if(dot(v, m) * v.v.z <= 0.f) { return 0.f; }

    float cos_square = square(v.v.z);
    float sin_square = 1.f - cos_square;
    float tan_square = sin_square / cos_square;

    switch(this->type) {
      case MicrofacetType::Beckmann: {
        float a = 1.f / (this->alpha * sqrt(tan_square));
        if(a >= 1.6f) { return 1.f; }
        return (3.535f*a + 2.181f*a*a) / (1.f + 2.276f*a + 2.577f*a*a);
      }
      case MicrofacetType::Phong: {
        float a = sqrt((0.5f * this->alpha + 1.f) / tan_square);
        if(a >= 1.6f) { return 1.f; }
        return (3.535f*a + 2.181f*a*a) / (1.f + 2.276f*a + 2.577f*a*a);
      }
      case MicrofacetType::GGX:
        return 2.f / (1.f + sqrt(1.f + square(this->alpha) * tan_square));
    }
    return 0.f;
  }

  float MicrofacetDistrib::g(const Vector& i, const Vector& o, const Vector& m) const {
    return this->g1(i, m) * this->g1(o, m);
  }

  Vector MicrofacetDistrib::sample_m(Vec2 uv, float& out_dir_pdf) const {
    float cos_theta = 0.f;
    switch(this->type) {
      case MicrofacetType::Beckmann:
        cos_theta = 1.f / sqrt(1.f - square(this->alpha) * log(uv.x));
      case MicrofacetType::Phong:
        cos_theta = pow(uv.x, 1.f / (this->alpha + 2.f));
      case MicrofacetType::GGX:
        cos_theta = 1.f / sqrt((square(this->alpha) * uv.x) / (1.f - uv.x) + 1.f);
    };

    assert(cos_theta <= 1.f);
    float sin_theta = sqrt(1.f - square(cos_theta));
    float sin_phi = sin(uv.y * TWO_PI);
    float cos_phi = cos(uv.y * TWO_PI);
    float x = sin_theta * sin_phi;
    float y = sin_theta * cos_phi;
    float z = cos_theta;
    out_dir_pdf = this->d(Vector(x, y, z)) * z;
    return Vector(x, y, z);
  }
}

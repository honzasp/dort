#pragma once
#include "dort/dort.hpp"

namespace dort {
  enum class MicrofacetType {
    Beckmann,
    Phong,
    GGX,
  };

  class MicrofacetDistrib final {
    MicrofacetType type;
    float alpha;
  public:
    MicrofacetDistrib(MicrofacetType type, float roughness);
    float d(const Vector& m) const;
    float g1(const Vector& v, const Vector& m) const;
    float g(const Vector& i, const Vector& o, const Vector& m) const;
    Vector sample_m(Vec2 uv, float& out_dir_pdf) const;
  };
}


#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/sampler.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  enum BxdfFlags: uint8_t {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
  };
  constexpr BxdfFlags BSDF_ALL_TYPES = BxdfFlags(
    BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR);
  constexpr BxdfFlags BSDF_ALL = BxdfFlags(
    BSDF_ALL_TYPES | BSDF_REFLECTION | BSDF_TRANSMISSION);

  struct Bxdf {
    BxdfFlags flags;

    Bxdf(BxdfFlags flags): flags(flags) { }
    virtual ~Bxdf() { }

    bool matches(BxdfFlags test) {
      return (this->flags & test) == this->flags;
    }

    virtual Spectrum f(const Vector& wo, const Vector& wi) const = 0;
    virtual Spectrum sample_f(const Vector& wo, Vector& out_wi,
        float& out_pdf, float u1, float u2) const = 0;
    virtual float f_pdf(const Vector& wo, const Vector& wi) const = 0;
  };

  struct BsdfSamplesIdxs {
    SampleIdx uv_pos_idx;
    SampleIdx u_component_idx;
    uint32_t count;
  };

  struct BsdfSample {
    Vec2 uv_pos;
    float u_component;

    explicit BsdfSample(Sampler& sampler);
    BsdfSample(Sampler& sampler, const BsdfSamplesIdxs& idxs, uint32_t n);
    static BsdfSamplesIdxs request(Sampler& sampler, uint32_t count);
  };

  class Bsdf {
    // TODO: use a small_vector
    std::vector<std::unique_ptr<Bxdf>> bxdfs;
    Vector nn;
    Vector sn;
    Vector tn;
    Normal nn_geom;
  public:
    Bsdf(const DiffGeom& diff_geom, const Normal& nn_geom);
    void add(std::unique_ptr<Bxdf> bxdf);

    Spectrum f(const Vector& wo, const Vector& wi, BxdfFlags flags) const;
    Spectrum sample_f(const Vector& wo, Vector& out_wi, float& out_pdf,
        BxdfFlags flags, BxdfFlags& out_flags, BsdfSample sample) const;
    float f_pdf(const Vector& wo, const Vector& wi, BxdfFlags flags) const;
    uint32_t num_bxdfs(BxdfFlags flags) const;
    uint32_t num_bxdfs() const { return this->bxdfs.size(); }

    Vector local_to_world(const Vector& vec) const {
      return this->sn * vec.v.x + this->tn * vec.v.y + this->nn * vec.v.z;
    }
    Vector world_to_local(const Vector& vec) const {
      return Vector(dot(this->sn, vec), dot(this->tn, vec), dot(this->nn, vec));
    }

    static bool same_hemisphere(const Vector& w1, const Vector& w2) {
      return w1.v.z * w2.v.z > 0.f;
    }

    static float cos_theta(const Vector& w) {
      return w.v.z;
    }
    static float cos_theta_square(const Vector& w) {
      return square(w.v.z);
    }
    static float abs_cos_theta(const Vector& w) {
      return abs(w.v.z);
    }
    static float sin_theta_square(const Vector& w) {
      return 1.f - square(w.v.z);
    }
    static float sin_theta(const Vector& w) {
      return sqrt(max(0.f, 1.f - square(w.v.z)));
    }

    static float sin_phi_square(const Vector& w) {
      return square(w.v.y) / sin_theta_square(w);
    }
    static float sin_phi(const Vector& w) {
      return w.v.y / sin_theta(w);
    }
    static float cos_phi_square(const Vector& w) {
      return square(w.v.x) / sin_theta_square(w);
    }
    static float cos_phi(const Vector& w) {
      return w.v.x / sin_theta(w);
    }
  };
}

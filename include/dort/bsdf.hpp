#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/sampler.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  enum BxdfFlags: uint8_t {
    BSDF_REFLECTION = 1,
      ///< The BSDF component scatters light in the incident hemisphere
    BSDF_TRANSMISSION = 2,
      ///< The BSDF component scatters light in the hemisphere opposite to the
      /// incident direction.

    BSDF_DIFFUSE = 4,
      ///< The BSDF scatters light in a large solid angle (the contribution of a
      /// random pair of directions is likely to be nonzero).
    BSDF_GLOSSY = 8,
      ///< The BSDF scatters light in a small solid angle (the contribution of a
      /// random pair of directions is likely to be very small or zero).
    BSDF_DELTA = 16,
      ///< The BSDF scatters light by a delta distribution (the contribution of
      /// a random pair of directions is always zero).

    BSDF_ALL_HEMISPHERES = BSDF_REFLECTION | BSDF_TRANSMISSION,
    BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_DELTA,
    BSDF_ALL = BSDF_ALL_HEMISPHERES | BSDF_ALL_TYPES,
  };

  constexpr BxdfFlags operator|(BxdfFlags a, BxdfFlags b) {
    return BxdfFlags(uint8_t(a) | uint8_t(b));
  }
  constexpr BxdfFlags operator&(BxdfFlags a, BxdfFlags b) {
    return BxdfFlags(uint8_t(a) & uint8_t(b));
  }
  constexpr BxdfFlags operator~(BxdfFlags a) {
    return BxdfFlags(~uint8_t(a));
  }


  struct Bxdf {
    BxdfFlags flags;

    Bxdf(BxdfFlags flags): flags(flags) { }
    virtual ~Bxdf() { }

    bool matches(BxdfFlags test) {
      return (this->flags & test) == this->flags;
    }

    virtual Spectrum eval_f(const Vector& wi_light, const Vector& wo_camera) const = 0;
    virtual Spectrum sample_light_f(const Vector& wo_camera,
        Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const = 0;
    virtual Spectrum sample_camera_f(const Vector& wi_light,
        Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const = 0;
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix) const = 0;
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix) const = 0;
  };

  struct SymmetricBxdf: public Bxdf {
    SymmetricBxdf(BxdfFlags flags): Bxdf(flags) { }

    virtual Spectrum sample_light_f(const Vector& wo_camera,
        Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const override final;
    virtual Spectrum sample_camera_f(const Vector& wi_light,
        Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const override final;
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix) const override final;
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix) const override final;

  protected:
    virtual Spectrum sample_symmetric_f(const Vector& w_fix,
        Vector& out_w_gen, float& out_dir_pdf, Vec2 uv) const = 0;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix) const = 0;
  };

  struct BsdfSamplesIdxs {
    SampleIdx uv_pos_idx;
    SampleIdx u_component_idx;
    uint32_t count;
  };

  struct BsdfSample {
    Vec2 uv_pos;
    float u_component;

    explicit BsdfSample(Rng& rng);
    BsdfSample(Sampler& sampler, const BsdfSamplesIdxs& idxs, uint32_t n);
    static BsdfSamplesIdxs request(Sampler& sampler, uint32_t count);
  };

  class Bsdf final {
    // TODO: use a small_vector
    std::vector<std::unique_ptr<Bxdf>> bxdfs;
    Vector nn;
    Vector sn;
    Vector tn;
    Normal nn_geom;
  public:
    Bsdf(const DiffGeom& diff_geom, const Normal& nn_geom);
    void add(std::unique_ptr<Bxdf> bxdf);

    /// Evaluates the BSDF for the pair of directions.
    /// Vector wi_light points to the light, while wo_camera points to the
    /// camera. Compontents that do not match flags are ignored. Delta
    /// components are ignored (even if the two directions match exactly).
    Spectrum eval_f(const Vector& wi_light, const Vector& wo_camera,
        BxdfFlags flags) const;

    /// Samples a direction to light.
    /// Given a direction to camera (wo_camera), samples a direction to light
    /// and returns the bsdf value in this direction. If a delta component is
    /// sampled, it is assumed to be evaluated under an integral over the
    /// wi_light direction and the return value and pdf are coefficients of the
    /// respective delta distribution 
    Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags flags,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;
    
    /// Samples a direction to camera.
    /// Given a direction to light (wi_light), samples a direction to camera and
    /// returns the bsdf value in this direction. The handling of delta
    /// components is the same as in sample_light_f().
    Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags flags,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;

    /// Computes the pdf of sample_light_f().
    /// Returns the probability of sampling the light direction wi_light_gen
    /// given wo_camera_fix from sample_light_f(). The pdf is w.r.t. solid
    /// angle. Delta distributions are ignored (even if the directions would
    /// match exactly).
    float light_f_pdf(const Vector& wi_light_gen, const Vector& wo_camera_fix,
        BxdfFlags flags) const;

    /// Computes the pdf of sample_camera_f().
    /// Returns the probability of sampling the camera direction wo_camera_gen
    /// given light direction wi_light_fix from sample_camera_f().
    float camera_f_pdf(const Vector& wo_camera_gen, const Vector& wi_light_fix,
        BxdfFlags flags) const;

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
  private:
    template<bool FIX_IS_CAMERA>
    Spectrum sample_f(const Vector& w_fix, BxdfFlags flags,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;

    template<bool FIX_IS_CAMERA>
    float f_pdf(const Vector& w_gen, const Vector& w_fix, BxdfFlags flags) const;
  };
}

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

    BSDF_MODES = BSDF_REFLECTION | BSDF_TRANSMISSION,
    BSDF_LOBES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_DELTA,
    BSDF_ALL = BSDF_MODES | BSDF_LOBES,
    BSDF_NONE = 0,
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


  /// Internal part of a BSDF.
  /// Bxdfs implement the scattering distributions. They can contain multiple
  /// components (for example, a single Bxdf can contain both reflection and
  /// transmission), in which case the flags must contain the union of all the
  /// flags.
  struct Bxdf {
    /// The union of all components that this BxDF can sample and evaluate.
    /// This is used to quickly skip BxDFs that cannot contribute to a sample or
    /// evaluation, but the BxDFs must also check the requested flags and apply
    /// only the matching components. However, the Bsdf guarantees that the Bxdf
    /// is evaluated only if matches_request returns true for the request.
    BxdfFlags flags;

    Bxdf(BxdfFlags flags): flags(flags) {
      assert(flags & BSDF_MODES);
      assert(flags & BSDF_LOBES);
    }
    virtual ~Bxdf() { }

    /// Returns true if there is a chance that the BxDF has a nonzero
    /// contribution for the given request. The BxDF must support at least one
    /// of the modes (reflection or transmission) and at least one lobe
    /// (diffuse, glossy or delta).
    bool matches_request(BxdfFlags request) const {
      return (this->flags & (request & BSDF_MODES)) &&
        (this->flags & (request & BSDF_LOBES));
    }

    /// Evaluates the requested components of the BxDF corresponding to the
    /// light and camera directions in the shading frame. The request is
    /// guaranteed to contain exacly one of BSDF_REFLECTION or BSDF_TRANSMISSION
    /// (determined in Bsdf::eval() by the geometric normal) and the BxDF should
    /// base the decision beteen reflection/transmission only of the request
    /// (this reduces artifacts when shading normals are used).
    virtual Spectrum eval_f(const Vector& wi_light, const Vector& wo_camera,
        BxdfFlags request) const = 0;

    /// Samples a direction to light, given a direction to camera.
    /// Only the components that are present in the request are permitted to be
    /// sampled (black Spectrum and zero pdf should be returned if the BxDF has
    /// no matching component). The sampled component is returned in out_flags,
    /// but the returned spectrum must include all components that match the
    /// request!
    virtual Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags request,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const = 0;

    /// Samples a direction to camera given a direction to light.
    /// See sample_light_f() for details.
    virtual Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags request,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const = 0;

    /// Computes the pdf of sampling a direction to light given a direction to
    /// camera. This pdf must match the distribution used by sample_light_f().
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix, BxdfFlags request) const = 0;

    /// Computes the pdf of sampling a direction to camera given a direction to
    /// light.
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix, BxdfFlags request) const = 0;
  };

  /// Bxdf that is the same for scattering from light and from camera.
  struct SymmetricBxdf: public Bxdf {
    SymmetricBxdf(BxdfFlags flags): Bxdf(flags) { }

    virtual Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags request,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const override final;
    virtual Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags request,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const override final;
    virtual float light_f_pdf(const Vector& wi_light_gen,
        const Vector& wo_camera_fix, BxdfFlags request) const override final;
    virtual float camera_f_pdf(const Vector& wo_camera_gen,
        const Vector& wi_light_fix, BxdfFlags request) const override final;

    virtual Spectrum sample_symmetric_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        Vec3 uvc) const = 0;
    virtual float symmetric_f_pdf(const Vector& w_gen,
        const Vector& w_fix, BxdfFlags request) const = 0;
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

  /// Bidirectional scattering distributon function.
  /// Bsdf defines the local shading coordinate system and wraps the internal
  /// BxDFs, which implement the scattering.
  class Bsdf final {
    // TODO: use a small_vector
    std::vector<std::unique_ptr<Bxdf>> bxdfs;
    Vector nn_geom;
    Vector nn_shading;
    Vector sn;
    Vector tn;
  public:
    Bsdf(const DiffGeom& diff_geom);
    void add(std::unique_ptr<Bxdf> bxdf);

    /// Evaluates the BSDF for the pair of directions.
    /// Vector wi_light points to the light, while wo_camera points to the
    /// camera. Compontents that do not match the requested flags are ignored. Delta
    /// components are ignored (even if the two directions match exactly).
    ///
    /// The returned value is expected to be used in the light scattering
    /// equation using the geometric normal, the conversion from shading to
    /// geometric normal is done in the Bsdf methods and should not be done by
    /// callers.
    Spectrum eval_f(const Vector& wi_light, const Vector& wo_camera,
        BxdfFlags request) const;

    /// Samples a direction to light.
    /// Given a direction to camera (wo_camera), samples a direction to light
    /// and returns the bsdf value in this direction. If a delta component is
    /// sampled, it is assumed to be evaluated under an integral over the
    /// wi_light direction and the return value and pdf are coefficients of the
    /// respective delta distribution 
    Spectrum sample_light_f(const Vector& wo_camera, BxdfFlags request,
        Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;
    
    /// Samples a direction to camera.
    /// Given a direction to light (wi_light), samples a direction to camera and
    /// returns the bsdf value in this direction. The handling of delta
    /// components is the same as in sample_light_f().
    Spectrum sample_camera_f(const Vector& wi_light, BxdfFlags request,
        Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;

    /// Computes the pdf of sample_light_f().
    /// Returns the probability of sampling the light direction wi_light_gen
    /// given wo_camera_fix from sample_light_f(). The pdf is w.r.t. solid
    /// angle. Delta distributions are ignored (even if the directions would
    /// match exactly).
    float light_f_pdf(const Vector& wi_light_gen, const Vector& wo_camera_fix,
        BxdfFlags request) const;

    /// Computes the pdf of sample_camera_f().
    /// Returns the probability of sampling the camera direction wo_camera_gen
    /// given light direction wi_light_fix from sample_camera_f().
    float camera_f_pdf(const Vector& wo_camera_gen, const Vector& wi_light_fix,
        BxdfFlags request) const;

    /// Returns the number of BxDFs that can match the request flags.
    uint32_t bxdf_count(BxdfFlags request) const;

    /// Returns the total number of BxDFs.
    uint32_t bxdf_count() const {
      return this->bxdfs.size();
    }

    Vector local_to_world(const Vector& vec) const {
      return this->sn * vec.v.x + this->tn * vec.v.y + this->nn_shading * vec.v.z;
    }
    Vector world_to_local(const Vector& vec) const {
      return Vector(dot(this->sn, vec), dot(this->tn, vec), dot(this->nn_shading, vec));
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
    Spectrum sample_f(const Vector& w_fix, BxdfFlags request,
        Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
        BsdfSample sample) const;

    template<bool FIX_IS_CAMERA>
    float f_pdf(const Vector& w_gen, const Vector& w_fix, BxdfFlags request) const;

    float shading_to_geom(const Vector& wi_light) const;
  };
}

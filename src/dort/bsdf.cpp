#include "dort/bsdf.hpp"
#include "dort/rng.hpp"
#include "dort/shape.hpp"

namespace dort {
  Bsdf::Bsdf(const DiffGeom& diff_geom):
    nn(diff_geom.nn.v)
  {
    float len = length(diff_geom.dpdu);
    if(len < 1e-9) {
      coordinate_system(this->nn, this->sn, this->tn);
    } else {
      this->sn = diff_geom.dpdu / len;
      this->tn = cross(this->nn, this->sn);
    }
  }

  void Bsdf::add(std::unique_ptr<Bxdf> bxdf) {
    this->bxdfs.push_back(std::move(bxdf));
  }

  Spectrum Bsdf::f(const Vector& wo, const Vector& wi, BxdfFlags flags) const {
    Vector wo_local = this->world_to_local(wo);
    Vector wi_local = this->world_to_local(wi);

    if(Bsdf::same_hemisphere(wo_local, wi_local)) {
      flags = BxdfFlags(flags & ~BSDF_TRANSMISSION);
    } else {
      flags = BxdfFlags(flags & ~BSDF_REFLECTION);
    }

    Spectrum f;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags)) {
        f = f + bxdf->f(wo_local, wi_local);
      }
    }
    return f;
  }

  Spectrum Bsdf::sample_f(const Vector& wo, Vector& out_wi, float& out_pdf,
      BxdfFlags flags, BxdfFlags& out_flags, Rng& rng) const
  {
    uint32_t num_bxdfs = this->num_bxdfs(flags);
    if(num_bxdfs == 0) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }
      
    uint32_t bxdf_idx = floor_int32(float(num_bxdfs) * rng.uniform_float());
    const Bxdf* sampled_bxdf = this->bxdfs.at(0).get();
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags) && (bxdf_idx--) == 0) {
        sampled_bxdf = bxdf.get();
        break;
      }
    }

    float u1 = rng.uniform_float();
    float u2 = rng.uniform_float();
    Vector wo_local = this->world_to_local(wo);
    Vector wi_local;
    Spectrum f = sampled_bxdf->sample_f(wo_local, wi_local, out_pdf, u1, u2);
    out_flags = sampled_bxdf->flags;
    out_wi = this->local_to_world(wi_local);
    return f;
  }

  float Bsdf::f_pdf(const Vector& wo, const Vector& wi, BxdfFlags flags) const {
    Vector wo_local = this->world_to_local(wo);
    Vector wi_local = this->world_to_local(wi);

    float sum_pdf = 0.f;
    uint32_t num_bxdfs = 0;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags)) {
        sum_pdf = sum_pdf + bxdf->f_pdf(wo_local, wi_local);
        num_bxdfs = num_bxdfs + 1;
      }
    }
    return sum_pdf / float(num_bxdfs);
  }

  uint32_t Bsdf::num_bxdfs(BxdfFlags flags) const {
    uint32_t count = 0;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags)) {
        count = count + 1;
      }
    }
    return count;
  }
}

#include "dort/bsdf.hpp"
#include "dort/rng.hpp"
#include "dort/shape.hpp"

namespace dort {
  BsdfSample::BsdfSample(Sampler& sampler) {
    this->uv_pos = sampler.random_2d();
    this->u_component = sampler.random_1d();
  }

  BsdfSample::BsdfSample(Sampler& sampler, const BsdfSamplesIdxs& idxs, uint32_t n) {
    this->uv_pos = sampler.get_array_2d(idxs.uv_pos_idx).at(n);
    this->u_component = sampler.get_array_1d(idxs.u_component_idx).at(n);
  }

  BsdfSamplesIdxs BsdfSample::request(Sampler& sampler, uint32_t count) {
    BsdfSamplesIdxs idxs;
    idxs.uv_pos_idx = sampler.request_array_2d(count);
    idxs.u_component_idx = sampler.request_array_1d(count);
    idxs.count = count;
    return idxs;
  }

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

    Spectrum f_sum;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags)) {
        Spectrum f = bxdf->f(wo_local, wi_local);
        assert(is_finite(f) && is_nonnegative(f));
        f_sum += f;
      }
    }
    return f_sum;
  }

  Spectrum Bsdf::sample_f(const Vector& wo, Vector& out_wi, float& out_pdf,
      BxdfFlags flags, BxdfFlags& out_flags, BsdfSample sample) const
  {
    uint32_t num_bxdfs = this->num_bxdfs(flags);
    if(num_bxdfs == 0) {
      out_pdf = 0.f;
      return Spectrum(0.f);
    }
      
    uint32_t bxdf_idx = floor_int32(float(num_bxdfs) * sample.u_component);
    const Bxdf* sampled_bxdf = this->bxdfs.at(0).get();
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags) && (bxdf_idx--) == 0) {
        sampled_bxdf = bxdf.get();
        break;
      }
    }

    Vector wo_local = this->world_to_local(wo);
    Vector wi_local;
    float sampled_pdf;
    Spectrum sampled_f = sampled_bxdf->sample_f(wo_local, wi_local, sampled_pdf,
      sample.uv_pos.x, sample.uv_pos.y);

    out_wi = this->local_to_world(wi_local);
    out_flags = sampled_bxdf->flags;

    if(sampled_bxdf->flags & BSDF_SPECULAR || num_bxdfs == 1) {
      out_pdf = sampled_pdf;
      return sampled_f;
    }

    BxdfFlags eval_flags;
    if(Bsdf::same_hemisphere(wo_local, wi_local)) {
      eval_flags = BxdfFlags(flags & ~BSDF_TRANSMISSION);
    } else {
      eval_flags = BxdfFlags(flags & ~BSDF_REFLECTION);
    }

    float sum_pdfs = sampled_pdf;
    Spectrum sum_f = sampled_f;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf.get() == sampled_bxdf) {
        continue;
      }
      if(bxdf->matches(eval_flags)) {
        Spectrum f = bxdf->f(wo_local, wi_local);
        assert(is_finite(f) && is_nonnegative(f));
        sum_f += f;
      }
      if(bxdf->matches(flags)) {
        float pdf = bxdf->f_pdf(wo_local, wi_local);
        assert(is_finite(pdf) && pdf >= 0.f);
        sum_pdfs += pdf;
      }
    }

    out_pdf = sum_pdfs / float(num_bxdfs);
    return sum_f;
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

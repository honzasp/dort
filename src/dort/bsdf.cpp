#include "dort/bsdf.hpp"
#include "dort/rng.hpp"
#include "dort/shape.hpp"

namespace dort {
  BsdfSample::BsdfSample(Rng& rng) {
    this->uv_pos = Vec2(rng.uniform_float(), rng.uniform_float());
    this->u_component = rng.uniform_float();
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

  Spectrum SymmetricBxdf::sample_light_f(const Vector& wo_camera,
      Vector& out_wi_light, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_symmetric_f(wo_camera, out_wi_light, out_dir_pdf, uv);
  }

  Spectrum SymmetricBxdf::sample_camera_f(const Vector& wi_light,
      Vector& out_wo_camera, float& out_dir_pdf, Vec2 uv) const
  {
    return this->sample_symmetric_f(wi_light, out_wo_camera, out_dir_pdf, uv);
  }

  float SymmetricBxdf::light_f_pdf(const Vector& wi_light_gen,
      const Vector& wo_camera_fix) const
  {
    return this->symmetric_f_pdf(wi_light_gen, wo_camera_fix);
  }

  float SymmetricBxdf::camera_f_pdf(const Vector& wo_camera_gen,
      const Vector& wi_light_fix) const
  {
    return this->symmetric_f_pdf(wo_camera_gen, wi_light_fix);
  }

  Bsdf::Bsdf(const DiffGeom& diff_geom):
    nn_geom(diff_geom.nn.v),
    nn_shading(diff_geom.nn_shading.v)
  {
    float len = length(diff_geom.dpdu_shading);
    if(len < 1e-9) {
      coordinate_system(this->nn_shading, this->sn, this->tn);
    } else {
      this->sn = diff_geom.dpdu_shading / len;
      assert(abs_dot(this->sn, this->nn_shading) < 1e-3);
      this->tn = cross(this->nn_shading, this->sn);
    }
  }

  void Bsdf::add(std::unique_ptr<Bxdf> bxdf) {
    this->bxdfs.push_back(std::move(bxdf));
  }

  Spectrum Bsdf::eval_f(const Vector& wi_light, const Vector& wo_camera,
      BxdfFlags flags) const
  {
    Vector wi_local = this->world_to_local(wi_light);
    Vector wo_local = this->world_to_local(wo_camera);
    bool reflection = Bsdf::same_hemisphere(wo_local, wi_local);

    Spectrum f_sum;
    for(const auto& bxdf: this->bxdfs) {
      if(!bxdf->matches(flags)) { continue; }
      if(bxdf->flags & (reflection ? BSDF_REFLECTION : BSDF_TRANSMISSION)) {
        Spectrum f = bxdf->eval_f(wi_local, wo_local);
        assert(is_finite(f) && is_nonnegative(f));
        f_sum += f;
      }
    }

    return f_sum * this->shading_to_geom(wi_light);
  }

  Spectrum Bsdf::sample_light_f(const Vector& wo_camera, BxdfFlags flags,
      Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    Spectrum bsdf_f = this->sample_f<true>(wo_camera, flags, out_wi_light,
        out_dir_pdf, out_flags, sample);
    return bsdf_f * this->shading_to_geom(out_wi_light);
  }

  Spectrum Bsdf::sample_camera_f(const Vector& wi_light, BxdfFlags flags,
      Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    Spectrum bsdf_f = this->sample_f<false>(wi_light, flags, out_wo_camera,
        out_dir_pdf, out_flags, sample);
    return bsdf_f * this->shading_to_geom(wi_light);
  }

  template<bool FIX_IS_CAMERA>
  Spectrum Bsdf::sample_f(const Vector& w_fix, BxdfFlags flags,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    out_dir_pdf = 0.f;
    out_flags = BxdfFlags(0);

    uint32_t num_bxdfs = this->num_bxdfs(flags);
    if(num_bxdfs == 0) { return Spectrum(0.f); }

    uint32_t bxdf_idx = floor_int32(float(num_bxdfs) * sample.u_component);
    const Bxdf* sampled_bxdf = this->bxdfs.at(0).get();
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags) && (bxdf_idx--) == 0) {
        sampled_bxdf = bxdf.get();
        break;
      }
    }

    Vector w_fix_local = this->world_to_local(w_fix);
    Vector w_gen_local;
    float sampled_dir_pdf;
    Spectrum sampled_f = FIX_IS_CAMERA
      ? sampled_bxdf->sample_light_f(w_fix_local, w_gen_local,
          sampled_dir_pdf, sample.uv_pos)
      : sampled_bxdf->sample_camera_f(w_fix_local, w_gen_local,
          sampled_dir_pdf, sample.uv_pos);
    assert(is_finite(sampled_f) && is_nonnegative(sampled_f));

    if(sampled_dir_pdf == 0.f) { return Spectrum(0.f); }
    assert(is_finite(sampled_dir_pdf) && sampled_dir_pdf >= 0.f);
    out_flags = sampled_bxdf->flags;
    out_w_gen = this->local_to_world(w_gen_local);

    if(num_bxdfs == 1) {
      out_dir_pdf = sampled_dir_pdf;
      return sampled_f;
    } else if(sampled_bxdf->flags & BSDF_DELTA) {
      out_dir_pdf = sampled_dir_pdf / float(num_bxdfs);
      return sampled_f;
    }

    bool reflection = Bsdf::same_hemisphere(w_fix_local, w_gen_local);

    float sum_dir_pdfs = sampled_dir_pdf;
    Spectrum sum_f = sampled_f;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf.get() == sampled_bxdf || !bxdf->matches(flags)) {
        continue;
      }
      if(bxdf->flags & (reflection ? BSDF_REFLECTION : BSDF_TRANSMISSION)) {
        Spectrum f = bxdf->eval_f(
            FIX_IS_CAMERA ? w_gen_local : w_fix_local,
            FIX_IS_CAMERA ? w_fix_local : w_gen_local);
        assert(is_finite(f) && is_nonnegative(f));
        sum_f += f;
      }

      float pdf = FIX_IS_CAMERA
        ? bxdf->light_f_pdf(w_gen_local, w_fix_local)
        : bxdf->camera_f_pdf(w_gen_local, w_fix_local);
      assert(is_finite(pdf) && pdf >= 0.f);
      sum_dir_pdfs += pdf;
    }

    out_dir_pdf = sum_dir_pdfs / float(num_bxdfs);
    return sum_f;
  }

  float Bsdf::light_f_pdf(const Vector& wi_light_gen, const Vector& wo_camera_fix,
      BxdfFlags flags) const
  {
    return this->f_pdf<true>(wi_light_gen, wo_camera_fix, flags);
  }

  float Bsdf::camera_f_pdf(const Vector& wo_camera_gen, const Vector& wi_light_fix,
      BxdfFlags flags) const
  {
    return this->f_pdf<false>(wo_camera_gen, wi_light_fix, flags);
  }

  template<bool FIX_IS_CAMERA>
  float Bsdf::f_pdf(const Vector& w_gen, const Vector& w_fix, BxdfFlags flags) const {
    Vector w_gen_local = this->world_to_local(w_gen);
    Vector w_fix_local = this->world_to_local(w_fix);

    float sum_dir_pdfs = 0.f;
    uint32_t num_bxdfs = 0;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches(flags)) {
        float dir_pdf = FIX_IS_CAMERA 
          ? bxdf->light_f_pdf(w_gen_local, w_fix_local)
          : bxdf->camera_f_pdf(w_gen_local, w_fix_local);
        sum_dir_pdfs += dir_pdf;
        num_bxdfs += 1;
      }
    }

    return num_bxdfs > 0 ? (sum_dir_pdfs / float(num_bxdfs)) : 0.f;
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

  float Bsdf::shading_to_geom(const Vector& wi_light) const {
    float dot_shading = abs_dot(this->nn_shading, wi_light);
    float dot_geom = abs_dot(this->nn_geom, wi_light);
    if(dot_shading * dot_geom == 0.f) { return 0.f; }
    return dot_shading / dot_geom;
  }
}

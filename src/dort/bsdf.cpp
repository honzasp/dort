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

  Spectrum SymmetricBxdf::sample_light_f(const Vector& wo_camera, BxdfFlags request,
      Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags, Vec3 uvc) const
  {
    return this->sample_symmetric_f(wo_camera, request,
        out_wi_light, out_dir_pdf, out_flags, uvc);
  }

  Spectrum SymmetricBxdf::sample_camera_f(const Vector& wi_light, BxdfFlags request,
      Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags, Vec3 uvc) const
  {
    return this->sample_symmetric_f(wi_light, request,
        out_wo_camera, out_dir_pdf, out_flags, uvc);
  }

  float SymmetricBxdf::light_f_pdf(const Vector& wi_light_gen,
      const Vector& wo_camera_fix, BxdfFlags request) const
  {
    return this->symmetric_f_pdf(wi_light_gen, wo_camera_fix, request);
  }

  float SymmetricBxdf::camera_f_pdf(const Vector& wo_camera_gen,
      const Vector& wi_light_fix, BxdfFlags request) const
  {
    return this->symmetric_f_pdf(wo_camera_gen, wi_light_fix, request);
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
    assert(is_unit(this->nn_geom));
    assert(is_unit(this->nn_shading));
    assert(is_unit(this->sn));
    assert(is_unit(this->tn));
  }

  void Bsdf::add(std::unique_ptr<Bxdf> bxdf) {
    this->bxdfs.push_back(std::move(bxdf));
  }

  Spectrum Bsdf::eval_f(const Vector& wi_light, const Vector& wo_camera,
      BxdfFlags request) const
  {
    assert(is_unit(wi_light)); assert(is_unit(wo_camera));
    Vector wi_local = this->world_to_local(wi_light);
    Vector wo_local = this->world_to_local(wo_camera);
    bool reflection = Bsdf::same_hemisphere(wo_local, wi_local);
    BxdfFlags eval_request = request & ((BSDF_LOBES & (~BSDF_DELTA)) |
         (reflection ? BSDF_REFLECTION : BSDF_TRANSMISSION));
    if(!(eval_request & BSDF_MODES)) { return Spectrum(0.f); }

    Spectrum f_sum;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches_request(eval_request)) {
        Spectrum f = bxdf->eval_f(wi_local, wo_local, eval_request);
        assert(is_finite(f)); assert(is_nonnegative(f));
        f_sum += f;
      }
    }

    return f_sum * this->shading_to_geom(wi_light);
  }

  Spectrum Bsdf::sample_light_f(const Vector& wo_camera, BxdfFlags request,
      Vector& out_wi_light, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    Spectrum bsdf_f = this->sample_f<true>(wo_camera, request,
        out_wi_light, out_dir_pdf, out_flags, sample);
    return bsdf_f * this->shading_to_geom(out_wi_light);
  }

  Spectrum Bsdf::sample_camera_f(const Vector& wi_light, BxdfFlags request,
      Vector& out_wo_camera, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    Spectrum bsdf_f = this->sample_f<false>(wi_light, request,
        out_wo_camera, out_dir_pdf, out_flags, sample);
    return bsdf_f * this->shading_to_geom(wi_light);
  }

  template<bool FIX_IS_CAMERA>
  Spectrum Bsdf::sample_f(const Vector& w_fix, BxdfFlags request,
      Vector& out_w_gen, float& out_dir_pdf, BxdfFlags& out_flags,
      BsdfSample sample) const
  {
    assert(is_unit(w_fix));
    out_dir_pdf = 0.f;
    out_flags = BxdfFlags(0);

    uint32_t bxdf_count = this->bxdf_count(request);
    if(bxdf_count == 0) { return Spectrum(0.f); }

    uint32_t bxdf_idx = floor_int32(float(bxdf_count) * sample.u_component);
    sample.u_component = sample.u_component * float(bxdf_count) - float(bxdf_idx);
    const Bxdf* sampled_bxdf = this->bxdfs.at(0).get();
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches_request(request) && (bxdf_idx--) == 0) {
        sampled_bxdf = bxdf.get();
        break;
      }
    }

    Vec3 sample_uvc(sample.uv_pos.x, sample.uv_pos.y, sample.u_component);
    Vector w_fix_local = this->world_to_local(w_fix);
    Vector w_gen_local;
    float sampled_dir_pdf;
    BxdfFlags sampled_flags;
    Spectrum sampled_f = FIX_IS_CAMERA
      ? sampled_bxdf->sample_light_f(w_fix_local, request,
          w_gen_local, sampled_dir_pdf, sampled_flags, sample_uvc)
      : sampled_bxdf->sample_camera_f(w_fix_local, request,
          w_gen_local, sampled_dir_pdf, sampled_flags, sample_uvc);

    assert(is_finite(sampled_f)); assert(is_nonnegative(sampled_f));
    if(sampled_dir_pdf == 0.f) { return Spectrum(0.f); }
    assert(is_finite(sampled_dir_pdf)); assert(sampled_dir_pdf >= 0.f);
    assert((sampled_flags & BSDF_MODES)); assert((sampled_flags & BSDF_LOBES));
    assert(is_unit(w_gen_local));

    out_flags = sampled_flags;
    out_w_gen = this->local_to_world(w_gen_local);

    if(bxdf_count == 1) {
      out_dir_pdf = sampled_dir_pdf;
      return sampled_f;
    } else if(sampled_flags & BSDF_DELTA) {
      out_dir_pdf = sampled_dir_pdf / float(bxdf_count);
      return sampled_f;
    }

    bool reflection = Bsdf::same_hemisphere(w_fix_local, w_gen_local);
    BxdfFlags eval_request = request & ((BSDF_LOBES & (~BSDF_DELTA)) |
      (reflection ? BSDF_REFLECTION : BSDF_TRANSMISSION));

    float sum_dir_pdfs = sampled_dir_pdf;
    Spectrum sum_f = sampled_f;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf.get() == sampled_bxdf || !bxdf->matches_request(request)) {
        continue;
      }

      if(bxdf->matches_request(eval_request)) {
        Spectrum f = bxdf->eval_f(
            FIX_IS_CAMERA ? w_gen_local : w_fix_local,
            FIX_IS_CAMERA ? w_fix_local : w_gen_local,
            eval_request);
        assert(is_finite(f)); assert(is_nonnegative(f));
        sum_f += f;
      }

      float pdf = FIX_IS_CAMERA
        ? bxdf->light_f_pdf(w_gen_local, w_fix_local, request)
        : bxdf->camera_f_pdf(w_gen_local, w_fix_local, request);
      assert(is_finite(pdf)); assert(pdf >= 0.f);
      sum_dir_pdfs += pdf;
    }

    out_dir_pdf = sum_dir_pdfs / float(bxdf_count);
    return sum_f;
  }

  float Bsdf::light_f_pdf(const Vector& wi_light_gen, const Vector& wo_camera_fix,
      BxdfFlags request) const
  {
    return this->f_pdf<true>(wi_light_gen, wo_camera_fix, request);
  }

  float Bsdf::camera_f_pdf(const Vector& wo_camera_gen, const Vector& wi_light_fix,
      BxdfFlags request) const
  {
    return this->f_pdf<false>(wo_camera_gen, wi_light_fix, request);
  }

  template<bool FIX_IS_CAMERA>
  float Bsdf::f_pdf(const Vector& w_gen, const Vector& w_fix, BxdfFlags request) const {
    assert(is_unit(w_gen)); assert(is_unit(w_fix));
    Vector w_gen_local = this->world_to_local(w_gen);
    Vector w_fix_local = this->world_to_local(w_fix);

    float sum_dir_pdfs = 0.f;
    uint32_t bxdf_count = 0;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches_request(request)) {
        float dir_pdf = FIX_IS_CAMERA 
          ? bxdf->light_f_pdf(w_gen_local, w_fix_local, request)
          : bxdf->camera_f_pdf(w_gen_local, w_fix_local, request);
        sum_dir_pdfs += dir_pdf;
        bxdf_count += 1;
      }
    }

    return bxdf_count > 0 ? (sum_dir_pdfs / float(bxdf_count)) : 0.f;
  }

  uint32_t Bsdf::bxdf_count(BxdfFlags request) const {
    uint32_t count = 0;
    for(const auto& bxdf: this->bxdfs) {
      if(bxdf->matches_request(request)) {
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

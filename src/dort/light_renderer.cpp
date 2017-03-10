#include "dort/ctx.hpp"
#include "dort/film.hpp"
#include "dort/light_renderer.hpp"
#include "dort/primitive.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  void LightRenderer::render(CtxG& ctx, Progress& progress) {
    StatTimer t(TIMER_RENDER);

    uint64_t path_count = uint64_t(this->iteration_count) *
      this->film->x_res * this->film->y_res;
    uint32_t job_count = this->get_job_count(ctx);

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    std::atomic<uint32_t> jobs_done(0);
    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      if(progress.is_cancelled()) { return; }
      uint64_t begin = uint64_t(job_i) * path_count / job_count;
      uint64_t end = uint64_t(job_i + 1) * path_count / job_count;
      auto& sampler = *samplers.at(job_i);

      for(uint64_t i = begin; i < end; ++i) {
        Vec2 film_pos;
        Spectrum contrib = this->sample_path(sampler, film_pos);
        this->film->add_splat(film_pos, contrib);
      }

      uint32_t done = jobs_done.fetch_add(1) + 1;
      progress.set_percent_done(float(done) / float(job_count));
      {
        std::unique_lock<std::mutex> film_lock(film_mutex);
        this->film->splat_scale = float(job_count) /
          (float(done) * float(this->iteration_count));
      }
    });

    this->film->splat_scale = 1.f / float(this->iteration_count);
    (void)this->max_depth;
  }

  Spectrum LightRenderer::sample_path(Sampler& sampler, Vec2& out_film_pos) const {
    out_film_pos = sampler.random_2d() * Vec2(this->film->x_res, this->film->y_res);
    return Spectrum(0.3f);
  }

  uint32_t LightRenderer::get_job_count(const CtxG& ctx) const {
    return max(ctx.pool->num_threads() * 16, this->iteration_count);
  }
}

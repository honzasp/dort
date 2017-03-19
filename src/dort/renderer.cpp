#include "dort/ctx.hpp"
#include "dort/film.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/thread_pool.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  Vec2i Renderer::layout_tiles(const CtxG& ctx,
      const Film& film, const Sampler& sampler)
  {
    uint32_t min_jobs_per_thread = 16;
    uint32_t max_samples_per_job = 32*1024;
    uint32_t approx_jobs = max(1u, max(
        ctx.pool->num_threads() * min_jobs_per_thread,
        film.x_res * film.y_res * sampler.samples_per_pixel / max_samples_per_job));

    uint32_t tiles_x = round_up_power_of_two(approx_jobs);
    uint32_t tiles_y = 1;
    while(film.x_res * tiles_y < film.y_res * tiles_x) {
      tiles_x = tiles_x / 2;
      tiles_y = tiles_y * 2;
    }

    return Vec2i(tiles_x, tiles_y);
  }

  void Renderer::render_tiled(CtxG& ctx, Progress& progress,
      uint32_t iteration_count,
      std::function<void(CtxG&,Recti,Recti,Film&,Sampler&,Progress&)> render_tile,
      std::function<void(Film&,uint32_t)> iteration_callback)
  {
    StatTimer t(TIMER_RENDER);
    Vec2i layout_tiles = Renderer::layout_tiles(ctx, *this->film, *this->sampler);
    Vec2 tile_size = Vec2(float(this->film->x_res), float(this->film->y_res)) /
      Vec2(float(layout_tiles.x), float(layout_tiles.y));
    uint32_t job_count = layout_tiles.x * layout_tiles.y * iteration_count;
    stat_sample_int(DISTRIB_INT_RENDER_JOBS, job_count);

    // TODO: it is wasteful to produce so many samplers
    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    std::atomic<uint32_t> jobs_done(0);
    std::atomic<uint32_t> iters_done(0);
    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      if(progress.is_cancelled()) { return; }

      uint32_t iter_job_i = job_i % (layout_tiles.x * layout_tiles.y);
      if(iter_job_i == 0) {
        uint32_t iteration = iters_done.fetch_add(1);
        std::unique_lock<std::mutex> film_lock(film_mutex);
        iteration_callback(*this->film, iteration);
      }

      uint32_t tile_x = iter_job_i % layout_tiles.x;
      uint32_t tile_y = iter_job_i / layout_tiles.x;
      Vec2 corner_0f = tile_size * Vec2(float(tile_x), float(tile_y));
      Vec2 corner_1f = corner_0f + tile_size;
      Recti tile_rect(floor_vec2i(corner_0f), floor_vec2i(corner_1f));

      Vec2 margin = this->film->filter.radius;
      Recti film_rect(floor_vec2i(corner_0f - margin), ceil_vec2i(corner_1f + margin));
      Vec2i film_size = film_rect.p_max - film_rect.p_min;
      Film tile_film(film_size.x, film_size.y, this->film->filter);
      render_tile(ctx, tile_rect, film_rect, tile_film,
        *samplers.at(job_i), progress);

      {
        std::unique_lock<std::mutex> film_lock(film_mutex);
        this->film->add_tile(film_rect.p_min, tile_film);
      }

      uint32_t done = jobs_done.fetch_add(1) + 1;
      progress.set_percent_done(float(done) / float(job_count));
    });
  }
}

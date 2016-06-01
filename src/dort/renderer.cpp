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
}

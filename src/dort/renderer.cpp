#include "dort/ctx.hpp"
#include "dort/film.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/thread_pool.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  Vec2i Renderer::layout_tiles(const CtxG& ctx, Vec2i film_res) {
    uint32_t min_jobs_per_thread = 8;
    uint32_t max_pixels_per_job = 32*1024;
    uint32_t approx_jobs = max(1u, max(
        ctx.pool->thread_count() * min_jobs_per_thread,
        film_res.x * film_res.y / max_pixels_per_job));

    uint32_t tiles_x = round_up_power_of_two(approx_jobs);
    uint32_t tiles_y = 1;
    while(film_res.x * tiles_y < film_res.y * tiles_x) {
      tiles_x = tiles_x / 2;
      tiles_y = tiles_y * 2;
    }

    return Vec2i(tiles_x, tiles_y);
  }

  void Renderer::iteration_tiled(CtxG& ctx,
      std::function<Spectrum(Vec2i, Vec2&, Sampler&)> get_pixel_contrib)
  {
    Vec2i layout_tiles = Renderer::layout_tiles(ctx, this->film->res);
    Vec2 tile_size = Vec2(this->film->res) /
      Vec2(float(layout_tiles.x), float(layout_tiles.y));
    uint32_t tile_count = layout_tiles.x * layout_tiles.y;

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < tile_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    std::atomic<uint32_t> jobs_done(0);
    std::atomic<uint32_t> iters_done(0);
    fork_join(*ctx.pool, tile_count, [&](uint32_t tile_i) {
      uint32_t tile_x = tile_i % layout_tiles.x;
      uint32_t tile_y = tile_i / layout_tiles.x;
      Vec2 corner_0f = tile_size * Vec2(float(tile_x), float(tile_y));
      Vec2 corner_1f = corner_0f + tile_size;
      Recti tile_rect(floor_vec2i(corner_0f), floor_vec2i(corner_1f));

      Vec2 margin = this->film->filter.radius;
      Recti film_rect(floor_vec2i(corner_0f - margin), ceil_vec2i(corner_1f + margin));
      Vec2i film_size = film_rect.p_max - film_rect.p_min;
      Film tile_film(film_size.x, film_size.y, this->film->filter);

      Sampler& sampler = *samplers.at(tile_i);
      for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
        for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
          Vec2 film_pos;
          Spectrum contrib = get_pixel_contrib(Vec2i(x, y), film_pos, sampler);
          assert(is_finite(contrib));
          assert(is_nonnegative(contrib));
          if(is_finite(contrib) && is_nonnegative(contrib)) {
            Vec2 tile_film_pos = film_pos - Vec2(film_rect.p_min);
            tile_film.add_sample(tile_film_pos, contrib);
          }
        }
      }

      {
        std::unique_lock<std::mutex> film_lock(film_mutex);
        this->film->add_tile(film_rect.p_min, tile_film);
      }
    });
  }
}

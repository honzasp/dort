#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/filter.hpp"
#include "dort/geometry.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  void Renderer::preprocess(CtxG&) {
    this->pixel_pos_idx = this->sampler->request_sample_2d();
    this->do_preprocess(*this->scene, *this->sampler);
  }

  void Renderer::render(CtxG& ctx) {
    StatTimer t(TIMER_RENDER);
    Vec2i layout_tiles = this->layout_tiles(ctx);
    Vec2 tile_size = Vec2(float(this->film->x_res), float(this->film->y_res)) /
      Vec2(float(layout_tiles.x), float(layout_tiles.y));
    uint32_t job_count = layout_tiles.x * layout_tiles.y;
    stat_sample_int(DISTRIB_INT_RENDER_JOBS, job_count);

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      uint32_t tile_x = job_i % layout_tiles.x;
      uint32_t tile_y = job_i / layout_tiles.x;
      Vec2 corner_0f = tile_size * Vec2(float(tile_x), float(tile_y));
      Vec2 corner_1f = corner_0f + tile_size;
      Recti tile_rect(floor_vec2i(corner_0f), floor_vec2i(corner_1f));

      Vec2 margin = this->film->filter.radius;
      Recti film_rect(floor_vec2i(corner_0f - margin), ceil_vec2i(corner_1f + margin));
      Vec2i film_size = film_rect.p_max - film_rect.p_min;
      Film tile_film(film_size.x, film_size.y, this->film->filter);
      this->render_tile(ctx, tile_rect, film_rect, tile_film, *samplers.at(job_i));

      std::unique_lock<std::mutex> film_lock(film_mutex);
      this->film->add_tile(film_rect.p_min, tile_film);
    });
  }

  void Renderer::render_tile(CtxG&, Recti tile_rect, 
      Recti tile_film_rect, Film& tile_film, Sampler& sampler) const
  {
    StatTimer t(TIMER_RENDER_TILE);

    float x_res = float(this->film->x_res);
    float y_res = float(this->film->y_res);
    float res = max(x_res, y_res);
    Vec2 ndc_scale = 2.f / Vec2(res, -res);
    Vec2 ndc_shift = Vec2(-x_res / res, y_res / res);

    for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
      for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
        {
          StatTimer t(TIMER_SAMPLER_START_PIXEL);
          sampler.start_pixel();
        }

        for(uint32_t s = 0; s < sampler.samples_per_pixel; ++s) {
          {
            StatTimer t(TIMER_SAMPLER_START_PIXEL_SAMPLE);
            sampler.start_pixel_sample();
          }

          Vec2 pixel_pos = sampler.get_sample_2d(this->pixel_pos_idx);
          Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
          Vec2 ndc_pos = film_pos * ndc_scale + ndc_shift;

          Ray ray(this->scene->camera->generate_ray(ndc_pos));
          Spectrum radiance = this->get_radiance(*this->scene, ray, 0, sampler);

          assert(is_finite(radiance));
          assert(is_nonnegative(radiance));
          if(is_finite(radiance) && is_nonnegative(radiance)) {
            Vec2 tile_film_pos = film_pos -
              Vec2(float(tile_film_rect.p_min.x), float(tile_film_rect.p_min.y));
            tile_film.add_sample(tile_film_pos, radiance);
          }
        }
      }
    }
  }

  Vec2i Renderer::layout_tiles(CtxG& ctx) const {
    uint32_t min_jobs_per_thread = 32;
    uint32_t max_samples_per_job = 1024;
    uint32_t approx_jobs = max(1u, max(
        ctx.pool->num_threads() * min_jobs_per_thread,
        this->film->x_res * this->film->y_res *
          this->sampler->samples_per_pixel / max_samples_per_job));

    uint32_t tiles_x = round_up_power_of_two(approx_jobs);
    uint32_t tiles_y = 1;
    while(this->film->x_res * tiles_y < this->film->y_res * tiles_x) {
      tiles_x = tiles_x / 2;
      tiles_y = tiles_y * 2;
    }

    return Vec2i(tiles_x, tiles_y);
  }
}

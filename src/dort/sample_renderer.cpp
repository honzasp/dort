#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/filter.hpp"
#include "dort/geometry.hpp"
#include "dort/light.hpp"
#include "dort/sample_renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  void SampleRenderer::render(CtxG& ctx, Progress& progress) {
    this->pixel_pos_idx = this->sampler->request_sample_2d();
    this->preprocess(ctx, *this->scene, *this->sampler);

    StatTimer t(TIMER_RENDER);
    Vec2i layout_tiles = Renderer::layout_tiles(ctx, *this->film, *this->sampler);
    Vec2 tile_size = Vec2(float(this->film->x_res), float(this->film->y_res)) /
      Vec2(float(layout_tiles.x), float(layout_tiles.y));
    uint32_t job_count = layout_tiles.x * layout_tiles.y;
    stat_sample_int(DISTRIB_INT_RENDER_JOBS, job_count);

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    std::atomic<uint32_t> jobs_done(0);
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
      this->render_tile(ctx, tile_rect, film_rect, tile_film,
        *samplers.at(job_i), progress);

      std::unique_lock<std::mutex> film_lock(film_mutex);
      this->film->add_tile(film_rect.p_min, tile_film);

      uint32_t done = jobs_done.fetch_add(1) + 1;
      progress.set_percent_done(float(done) / float(job_count));
    });
  }

  void SampleRenderer::render_tile(CtxG&, Recti tile_rect, Recti tile_film_rect,
      Film& tile_film, Sampler& sampler, Progress& progress) const
  {
    StatTimer t(TIMER_RENDER_TILE);
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    const Camera& camera = *this->scene->camera;

    for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
      for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
        sampler.start_pixel();
        for(uint32_t s = 0; s < sampler.samples_per_pixel; ++s) {
          sampler.start_pixel_sample();

          Vec2 pixel_pos = sampler.get_sample_2d(this->pixel_pos_idx);
          Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
          Ray camera_ray;
          float camera_pos_pdf, camera_dir_pdf;
          Spectrum importance = camera.sample_ray_importance(film_res, film_pos,
              camera_ray, camera_pos_pdf, camera_dir_pdf, CameraSample(sampler.rng));
          float camera_ray_pdf = camera_pos_pdf * camera_dir_pdf;

          Spectrum radiance = this->get_radiance(*this->scene, camera_ray, 0, sampler);
          assert(is_finite(radiance));
          assert(is_nonnegative(radiance));
          if(is_finite(radiance) && is_nonnegative(radiance)) {
            Vec2 tile_film_pos = film_pos -
              Vec2(float(tile_film_rect.p_min.x), float(tile_film_rect.p_min.y));
            tile_film.add_sample(tile_film_pos, radiance * importance / camera_ray_pdf);
          }
        }
      }
      if(progress.is_cancelled()) { return; }
    }
  }
}

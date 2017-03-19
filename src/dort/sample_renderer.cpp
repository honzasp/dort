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
    this->render_tiled(ctx, progress, this->iteration_count,
        [this](CtxG& ctx, Recti tile_rect, Recti tile_film_res,
            Film& file_film, Sampler& sampler, Progress& progress) 
        {
          this->render_tile(ctx, tile_rect, tile_film_res, file_film, sampler, progress);
        },
        [this](Film& film, uint32_t iter) {
          this->iteration(film, iter);
        });
    this->postprocess(ctx);
  }

  void SampleRenderer::render_tile(CtxG&, Recti tile_rect, Recti tile_film_rect,
      Film& tile_film, Sampler& sampler, Progress& progress) const
  {
    StatTimer t(TIMER_RENDER_TILE);
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    const Camera& camera = *this->camera;

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

          Spectrum radiance = this->get_radiance(*this->scene,
              camera_ray, film_pos, 0, sampler);
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

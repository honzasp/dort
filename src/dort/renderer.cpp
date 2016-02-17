#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/geometry.hpp"
#include "dort/renderer.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"

namespace dort {
  void Renderer::preprocess() {
    this->pixel_pos_idx = this->sampler->request_sample_2d();
    this->preprocess_(*this->scene, *this->sampler);
  }

  void Renderer::render() {
    float x_res = float(this->film->x_res);
    float y_res = float(this->film->y_res);
    float res = max(x_res, y_res);
    Vec2 ndc_scale = 2.f / Vec2(res, -res);
    Vec2 ndc_shift = Vec2(-x_res / res, y_res / res);

    for(uint32_t y = 0; y < this->film->y_res; ++y) {
      for(uint32_t x = 0; x < this->film->x_res; ++x) {
        this->sampler->start_pixel();
        for(uint32_t s = 0; s < this->sampler->samples_per_pixel; ++s) {
          this->sampler->start_pixel_sample();
          Vec2 pixel_pos = this->sampler->get_sample_2d(this->pixel_pos_idx);
          Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
          Vec2 ndc_pos = film_pos * ndc_scale + ndc_shift;

          Ray ray(this->scene->camera->generate_ray(ndc_pos));
          Spectrum radiance = this->get_radiance(*this->scene, ray, 0, *this->sampler);

          assert(is_finite(radiance));
          assert(is_nonnegative(radiance));
          if(is_finite(radiance) && is_nonnegative(radiance)) {
            this->film->add_sample(film_pos, radiance);
          }
        }
      }
    }
  }
}

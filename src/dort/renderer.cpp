#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/geometry.hpp"
#include "dort/renderer.hpp"
#include "dort/rng.hpp"
#include "dort/scene.hpp"

namespace dort {
  void Renderer::render(const Scene& scene, Film& film, Rng& rng) {
    Vec2 ndc_scale = 2.f / Vec2(float(film.x_res), -float(film.y_res));
    Vec2 ndc_shift = Vec2(-1.f, 1.f);

    for(uint32_t y = 0; y < film.y_res; ++y) {
      for(uint32_t x = 0; x < film.x_res; ++x) {
        Vec2 film_pos = Vec2(float(x), float(y));
        Vec2 ndc_pos = film_pos * ndc_scale + ndc_shift;

        Ray ray(scene.camera->generate_ray(ndc_pos));
        /*if((x + y * film.x_res) % 3234 == 0) {
          std::printf("%f,%f: ray %f,%f,%f, dir %f,%f,%f ndc %f,%f\n", 
              film_pos.x, film_pos.y,
              ray.orig.v.x, ray.orig.v.y, ray.orig.v.z,
              ray.dir.v.x, ray.dir.v.y, ray.dir.v.z,
              ndc_pos.x, ndc_pos.y);
        }*/
        Spectrum radiance = this->get_radiance(scene, ray, 0, rng);
        assert(is_finite(radiance));
        assert(is_nonnegative(radiance));
        /*if(!radiance.is_black()) {
          std::printf("%u %u: %f %f %f\n", x, y, 
              radiance.red(), radiance.green(), radiance.blue());
        }*/
        film.add_sample(film_pos, radiance);
      }
    }
  }
}

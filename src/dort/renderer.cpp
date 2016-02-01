#include "dort/renderer.hpp"

namespace dort {
  void Renderer::render(const Scene& scene, Film& film)
  {
    for(uint32_t y = 0; y < film.height; ++y) {
      for(uint32_t x = 0; x < film.width; ++x) {
        float world_x = float(x) - 0.5f * float(film.width);
        float world_y = float(y) - 0.5f * float(film.height);
        Ray ray(Point(world_x, world_y, -1.f), Vector(0.f, 0.f, 1.f));
        Spectrum radiance = this->get_radiance(scene, ray);
        assert(is_finite(radiance));
        assert(is_nonnegative(radiance));
        /*std::printf("%u %u: %f %f %f\n", x, y, 
            radiance.red(), radiance.green(), radiance.blue());*/
        film.add_sample(float(x), float(y), radiance);
      }
    }
  }
}

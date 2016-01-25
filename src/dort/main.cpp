#include <cstdio>
#include <memory>
#include <vector>
#include "dort/main.hpp"
#include "dort/sphere.hpp"

namespace dort {
  int main() {
    uint32_t img_width = 600;
    uint32_t img_height = 400;
    float scale = 0.1f;

    std::unique_ptr<Shape> shape(new Sphere(10.f));

    std::vector<float> image(img_width * img_height);
    for(uint32_t y = 0; y < img_height; ++y) {
      for(uint32_t x = 0; x < img_width; ++x) {
        float world_x = float(x) * scale - float(img_width) * 0.5f * scale;
        float world_y = float(y) * scale - float(img_height) * 0.5f * scale;

        Ray ray(Point(world_x, world_y, -10.f), Vector(0.f, 0.f, 1.f));
        Hit hit;
        float pixel;
        if(shape->hit(ray, hit)) {
          assert_2(is_finite(ray.dir));
          assert_2(is_finite(hit.normal));
          pixel = abs_dot(ray.dir, normalize(hit.normal));
        } else {
          pixel = 1.f;
        }

        image.at(y * img_width + x) = pixel;
      }
    }

    FILE* output = std::fopen("output.ppm", "w");
    std::fprintf(output, "P6 %u %u 255\n", img_width, img_height);
    for(float pixel: image) {
      uint8_t level = clamp(floor_int32(256.f * pixel), 0, 255);
      uint8_t rgb[3] = { level, level, level };
      std::fwrite(rgb, 3, 1, output);
    }
    std::fclose(output);

    return 0;
  }
}

int main() {
  return dort::main();
}

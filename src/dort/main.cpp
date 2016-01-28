#include <cstdio>
#include <memory>
#include <vector>
#include "dort/main.hpp"
#include "dort/geometric_primitive.hpp"
#include "dort/list_primitive.hpp"
#include "dort/transform_primitive.hpp"
#include "dort/triangle_mesh.hpp"
#include "dort/sphere.hpp"

namespace dort {
  int main() {
    uint32_t img_width = 600;
    uint32_t img_height = 400;
    float zoom = 0.1f;

    Transform cube_trans = identity()
      * translate(-2.f, 1.f, -5.f)
      * rotate_x(PI * 0.2) * rotate_y(PI * 0.1)
      * scale(6.f, 6.f, 6.f);

    TriangleMesh cube;
    cube.points = {
      cube_trans.apply(Point(-1.f, +1.f, -1.f)),
      cube_trans.apply(Point(+1.f, +1.f, -1.f)),
      cube_trans.apply(Point(+1.f, -1.f, -1.f)),
      cube_trans.apply(Point(-1.f, -1.f, -1.f)),
      cube_trans.apply(Point(-1.f, +1.f, +1.f)),
      cube_trans.apply(Point(+1.f, +1.f, +1.f)),
      cube_trans.apply(Point(+1.f, -1.f, +1.f)),
      cube_trans.apply(Point(-1.f, -1.f, +1.f)),
    };
    cube.vertices = {
      0, 1, 2,
      0, 2, 3,
      1, 6, 2,
      1, 5, 6,
      0, 5, 1,
      0, 4, 5,
      0, 3, 4,
      3, 7, 4,
      3, 2, 7,
      2, 6, 7,
      6, 5, 7,
      5, 4, 7,
    };

    auto sphere = std::make_shared<Sphere>(10.f);
    auto red = Spectrum::from_rgb(1.f, 0.f, 0.f);
    auto green = Spectrum::from_rgb(0.f, 1.f, 0.f);
    auto cyan = Spectrum::from_rgb(0.f, 1.f, 1.f);

    std::vector<std::unique_ptr<Primitive>> prims;
    prims.push_back(std::unique_ptr<Primitive>(
          new GeometricPrimitive(sphere, red)));
    prims.push_back(std::unique_ptr<Primitive>(
          new TransformPrimitive(
            scale(2.f, 1.f, 1.f) * translate(Vector(10.f, 3.f, -2.f)),
            std::unique_ptr<Primitive>(
              new GeometricPrimitive(sphere, green)))));
    for(uint32_t i = 0; i < 12; ++i) {
      prims.push_back(std::unique_ptr<Primitive>(
            new GeometricPrimitive(std::make_shared<Triangle>(&cube, i), cyan)));
    }

    std::unique_ptr<Primitive> root_prim(new ListPrimitive(std::move(prims)));

    std::vector<RgbSpectrum> image(img_width * img_height);
    for(uint32_t y = 0; y < img_height; ++y) {
      for(uint32_t x = 0; x < img_width; ++x) {
        float world_x = float(x) * zoom - float(img_width) * 0.5f * zoom;
        float world_y = float(y) * zoom - float(img_height) * 0.5f * zoom;

        Ray ray(Point(world_x, world_y, -10.f), Vector(0.f, 0.f, 1.f));
        Intersection isect;
        Spectrum pixel;
        if(root_prim->intersect(ray, isect)) {
          assert(is_finite(ray.dir));
          assert(is_finite(isect.diff_geom.nn));
          Spectrum color = isect.primitive->get_color(isect.diff_geom);
          Normal ray_nn = Normal(normalize(ray.dir));
          pixel = abs_dot(ray_nn, isect.diff_geom.nn) * color;
        } else {
          pixel = Spectrum::from_rgb(0.f, 0.f, 1.f);
        }

        image.at(y * img_width + x) = pixel;
      }
    }

    FILE* output = std::fopen("output.ppm", "w");
    std::fprintf(output, "P6 %u %u 255\n", img_width, img_height);
    for(Spectrum pixel: image) {
      uint8_t r_level = clamp(floor_int32(256.f * pixel.red()), 0, 255);
      uint8_t g_level = clamp(floor_int32(256.f * pixel.green()), 0, 255);
      uint8_t b_level = clamp(floor_int32(256.f * pixel.blue()), 0, 255);
      uint8_t rgb[3] = { r_level, g_level, b_level };
      std::fwrite(rgb, 3, 1, output);
    }
    std::fclose(output);

    return 0;
  }
}

int main() {
  return dort::main();
}

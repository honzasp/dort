#include <cstdio>
#include <memory>
#include <vector>
#include "dort/main.hpp"
#include "dort/bvh_primitive.hpp"
#include "dort/geometric_primitive.hpp"
#include "dort/list_primitive.hpp"
#include "dort/read_ply.hpp"
#include "dort/transform_primitive.hpp"
#include "dort/triangle_mesh.hpp"
#include "dort/sphere.hpp"

namespace dort {
  int main() {
    uint32_t img_width = 200;
    uint32_t img_height = 600;
    float zoom = 0.05f;

    auto red = Spectrum(1.f, 0.2f, 0.1f);

    TriangleMesh mesh;
    std::vector<Triangle> triangles;
    if(!read_ply(std::fopen("data/ketchup.ply", "r"), mesh, triangles)) {
      std::fprintf(stderr, "Could not open ketchup\n");
      return 1;
    }

    std::vector<std::unique_ptr<Primitive>> prims;
    for(auto& triangle: triangles) {
      prims.push_back(std::unique_ptr<Primitive>(
            new GeometricPrimitive(std::make_shared<Triangle>(triangle), red)));
    }

    std::printf("%lu primitives\n", prims.size());

    std::unique_ptr<Primitive> mesh_prim(new BvhPrimitive(
          std::move(prims), 4, BvhSplitMethod::Middle));
    std::unique_ptr<Primitive> root_prim(new TransformPrimitive(
            identity()
          * rotate_y(PI * 0.1f)
          * rotate_x(PI * 0.6f) 
          * scale(3.f, 3.f, 3.f)
          * translate(0.f, 0.f, -4.f),
          std::move(mesh_prim)));

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
      std::printf(".");
      std::fflush(stdout);
    }
    std::printf("\n");

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

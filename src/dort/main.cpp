#include <cstdio>
#include <memory>
#include <vector>
#include "dort/main.hpp"
#include "dort/bvh_primitive.hpp"
#include "dort/geometric_primitive.hpp"
#include "dort/list_primitive.hpp"
#include "dort/point_light.hpp"
#include "dort/read_ply.hpp"
#include "dort/transform_primitive.hpp"
#include "dort/triangle_mesh.hpp"
#include "dort/sphere.hpp"
#include "dort/whitted_renderer.hpp"

namespace dort {
  int main() {
    TriangleMesh mesh;
    std::vector<Triangle> triangles;
    if(!read_ply(std::fopen("data/dragon_vrip.ply", "r"), mesh, triangles)) {
      std::fprintf(stderr, "Could not open ply\n");
      return 1;
    }

    std::vector<std::unique_ptr<Primitive>> prims;
    for(auto& triangle: triangles) {
      prims.push_back(std::unique_ptr<Primitive>(
            new GeometricPrimitive(std::make_shared<Triangle>(triangle),
              Spectrum(0.8f, 0.8f, 0.8f))));
    }

    std::printf("%lu primitives\n", prims.size());

    std::unique_ptr<Primitive> mesh_prim(new BvhPrimitive(
          std::move(prims), 4, BvhSplitMethod::Middle));
    std::unique_ptr<Primitive> root_prim(new TransformPrimitive(
          rotate_x(0.1f * PI) * rotate_y(0.9f * PI) *
          translate(0.f, 300.f, 0.f) * scale(2.5e3f, -2.5e3f, 2.5e3f),
          std::move(mesh_prim)));

    Scene scene;
    scene.primitive = std::move(root_prim);
    scene.lights.push_back(std::unique_ptr<Light>(
          new PointLight(Point(0.f, 0.f, -400.f), 
          50000.f * Spectrum(1.f, 0.2f, 0.2f))));
    scene.lights.push_back(std::unique_ptr<Light>(
          new PointLight(Point(400.f, 0.f, 0.f),
          80000.f * Spectrum(1.f, 1.f, 0.f))));
    scene.lights.push_back(std::unique_ptr<Light>(
          new PointLight(Point(-400.f, -300.f, -200.f),
          150000.f * Spectrum(0.5f, 0.5f, 1.f))));

    Film film(800, 800);
    WhittedRenderer renderer(0);
    renderer.render(scene, film);

    FILE* output = std::fopen("output.ppm", "w");
    film.write_ppm(output);
    std::fclose(output);

    return 0;
  }
}

int main() {
  return dort::main();
}

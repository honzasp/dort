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

    std::vector<std::unique_ptr<Primitive>> mesh_prims;
    for(auto& triangle: triangles) {
      mesh_prims.push_back(std::make_unique<GeometricPrimitive>(
            std::make_shared<Triangle>(triangle),
            Spectrum(1.0f, 0.8f, 0.8f), 0.2f));
    }

    auto dragon_prim = std::make_unique<TransformPrimitive>(
      translate(-200.f, 200.f, -0.f) *
      rotate_x(0.1f * PI) * rotate_y(0.9f * PI) *
      translate(0.f, 300.f, 0.f) * scale(2.5e3f, -2.5e3f, 2.5e3f),
      std::make_unique<BvhPrimitive>(
            std::move(mesh_prims), 4, BvhSplitMethod::Middle));

    std::vector<std::unique_ptr<Primitive>> prims;
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(250.f, -100.f, 250.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(150.f),
          Spectrum(1.f, 1.f, 1.f), 0.2f)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(50.f, 150.f, 350.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(150.f),
          Spectrum(1.f, 1.f, 1.f), 1.0f)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(0.f, 0.f, 200.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(100.f),
          Spectrum(1.f, 1.f, 1.f), 0.7f)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(-150.f, 50.f, 150.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(100.f),
          Spectrum(1.f, 1.f, 1.f), 0.7f)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(0.f, 0.f, 0.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(100.f),
          Spectrum(1.f, 1.f, 1.f), 0.6f)));
    prims.push_back(std::move(dragon_prim));

    Scene scene;
    scene.primitive = std::make_unique<BvhPrimitive>(
        std::move(prims), 2, BvhSplitMethod::Middle);
    scene.lights.push_back(std::make_unique<PointLight>(
          Point(-800.f, 0.f, 0.f), 200000.f * Spectrum(1.f, 1.f, 1.f)));
    scene.lights.push_back(std::make_unique<PointLight>(
          Point(800.f, 0.f, 0.f), 200000.f * Spectrum(1.f, 1.f, 1.f)));
    scene.lights.push_back(std::make_unique<PointLight>(
          Point(0.f, 300.f, -600.f), 300000.f * Spectrum(1.f, 1.f, 1.f)));

    Film film(800, 800);
    WhittedRenderer renderer(10);
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

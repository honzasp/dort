#include <cstdio>
#include <memory>
#include <vector>
#include "dort/bvh_primitive.hpp"
#include "dort/diffuse_light.hpp"
#include "dort/direct_renderer.hpp"
#include "dort/disk.hpp"
#include "dort/lambertian_brdf.hpp"
#include "dort/list_primitive.hpp"
#include "dort/main.hpp"
#include "dort/matte_material.hpp"
#include "dort/point_light.hpp"
#include "dort/primitive.hpp"
#include "dort/read_ply.hpp"
#include "dort/specular_materials.hpp"
#include "dort/sphere.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  int main() {
    TriangleMesh mesh;
    std::vector<Triangle> triangles;
    if(!read_ply(std::fopen("data/dragon_vrip.ply", "r"), mesh, triangles)) {
      std::fprintf(stderr, "Could not open ply\n");
      return 1;
    }

    auto ply_material = std::make_shared<MatteMaterial>(
        Spectrum(1.f, 0.9f, 0.5f), 0.0f * PI);
    auto curtain_material = std::make_shared<MatteMaterial>(
        Spectrum(1.f, 1.f, 1.f));
    auto ball_material = std::make_shared<MatteMaterial>(
        Spectrum(0.5f, 0.9f, 1.0f));
    auto light_material = std::make_shared<MatteMaterial>(
        Spectrum(1.f, 1.f, 1.f));

    std::vector<std::unique_ptr<Primitive>> ply_prims;
    for(auto& triangle: triangles) {
      ply_prims.push_back(std::make_unique<GeometricPrimitive>(
            std::make_shared<Triangle>(triangle),
            ply_material));
    }

    auto ply_prim = std::make_unique<BvhPrimitive>(
        std::move(ply_prims), 4, BvhSplitMethod::Middle);
    auto ply_transform = 
      translate(15.f, 400.f, 0.f) *
      scale(3000.f) *
      rotate_x(1.1f * PI) *
      rotate_y(0.1f * PI);

    auto ball_prim = std::make_unique<GeometricPrimitive>(
        std::make_shared<Sphere>(100.f), ball_material);
    auto ball_transform =
      translate(-200.f, 0.f, 100.f);

    auto curtain_prim = std::make_unique<GeometricPrimitive>(
        std::make_shared<Disk>(1000.f), curtain_material);
    auto curtain_transform =
      translate(0.f, 0.f, 500.f) * rotate_x(0.1f * PI);

    auto light_shape = std::make_shared<Sphere>(60.f);
    auto light_transform = 
        translate(-200.f, 200.f, -1200.f) *
        rotate_y(0.3f * PI);
    auto area_light = std::make_shared<DiffuseLight>(light_shape,
        light_transform, 200.f * Spectrum(1.f), 1);
    auto light_prim = std::make_unique<GeometricPrimitive>(
        light_shape, light_material, area_light);

    std::vector<std::unique_ptr<Primitive>> prims;
    prims.push_back(std::make_unique<TransformPrimitive>(
      ply_transform, std::move(ply_prim)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      curtain_transform, std::move(curtain_prim)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      ball_transform, std::move(ball_prim)));
    /*prims.push_back(std::make_unique<TransformPrimitive>(
      light_transform, std::move(light_prim)));*/

    for(uint32_t i = 0; i < 10; ++i) {
      for(uint32_t j = 0; j < 10; ++j) {
        float u = float(i) / 10.f;
        float v = float(j) / 10.f;
        auto color = Spectrum(0.5f + u * 0.5f, 0.5f, 0.5f + v * 0.5f);
        auto material = std::make_shared<MatteMaterial>(color);
        auto transform = translate(
            -300.f + 600.f * u,
            -300.f + 600.f * v,
            100.f);
        prims.push_back(std::make_unique<TransformPrimitive>(
          transform, std::make_unique<GeometricPrimitive>(
            std::make_shared<Sphere>(10.f), material)));
      }
    }


    Scene scene;
    scene.primitive = std::make_unique<BvhPrimitive>(
        std::move(prims), 2, BvhSplitMethod::Middle);
    /*
    scene.lights.push_back(std::make_shared<PointLight>(
          Point(-1000.f, 0.f, 300.f), 7e5f * Spectrum(1.f, 1.f, 1.f)));
    scene.lights.push_back(std::make_shared<PointLight>(
          Point(1000.f, 0.f, 300.f), 7e5f * Spectrum(1.f, 1.f, 1.f)));
    scene.lights.push_back(std::make_shared<PointLight>(
          Point(0.f, -400.f, -900.f), 14e5f * Spectrum(1.f, 1.f, 1.f)));
          */
    scene.lights.push_back(area_light);

    Film film(800, 800);
    DirectRenderer renderer(5);
    Rng rng(42);
    renderer.render(scene, film, rng);

    FILE* output = std::fopen("output.ppm", "w");
    film.write_ppm(output);
    std::fclose(output);

    return 0;
  }
}

int main() {
  return dort::main();
}

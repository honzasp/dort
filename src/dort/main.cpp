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
#include "dort/material.hpp"
#include "dort/point_light.hpp"
#include "dort/primitive.hpp"
#include "dort/read_ply.hpp"
#include "dort/sphere.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  int main() {
    TriangleMesh mesh;
    std::vector<Triangle> triangles;
    if(!read_ply(std::fopen("data/cube.ply", "r"), mesh, triangles)) {
      std::fprintf(stderr, "Could not open ply\n");
      return 1;
    }

    auto mesh_material = std::make_shared<MatteMaterial>(
        Spectrum(1.f, 1.f, 1.f));
    std::vector<std::unique_ptr<Primitive>> mesh_prims;
    for(auto& triangle: triangles) {
      mesh_prims.push_back(std::make_unique<GeometricPrimitive>(
            std::make_shared<Triangle>(triangle),
            mesh_material));
    }

    auto ply_prim = std::make_unique<BvhPrimitive>(
          std::move(mesh_prims), 4, BvhSplitMethod::Middle);

    TriangleMesh light_mesh;
    light_mesh.points.push_back(Point(-260.f, -50.f, -120.f));
    light_mesh.points.push_back(Point(-255.f, +50.f, -80.f));
    light_mesh.points.push_back(Point(-250.f, -50.f, -60.f));
    light_mesh.vertices = { 2, 1, 0 };

    auto light_ball = std::make_shared<Triangle>(&light_mesh, 0);
    auto light_ball_transform = identity();
    auto ball_light = std::make_shared<DiffuseLight>(light_ball,
        light_ball_transform, 150.f * Spectrum(1.f, 1.f, 1.f), 5);

    auto light_disk = std::make_shared<Disk>(50.f);
    auto light_disk_transform = 
      translate(-200.f, 0.f, -200.f) * rotate_y(0.2 * PI);
    auto disk_light = std::make_shared<DiffuseLight>(light_disk,
        light_disk_transform, 50.f * Spectrum(0.f, 1.f, 1.f), 10);

    auto ball_material = std::make_shared<MatteMaterial>(
        Spectrum(1.f, 1.f, 1.f));
    std::vector<std::unique_ptr<Primitive>> prims;
    /*prims.push_back(std::make_unique<TransformPrimitive>(
      rotate_y(-0.25f * PI) * translate(0.f, 0.f, 300.f) * scale(200.f),
      std::move(ply_prim)));*/
    /*
    prims.push_back(std::make_unique<TransformPrimitive>(
      identity(),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(100.f), ball_material)));*/
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(400.f, 0.f, 400.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Sphere>(300.f), ball_material)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(0.f, 0.f, 200.f),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Disk>(600.f), ball_material)));
    prims.push_back(std::make_unique<TransformPrimitive>(
      translate(-100.f, 300.f, 0.f) * rotate_x(0.65f * PI),
      std::make_unique<GeometricPrimitive>(
          std::make_shared<Disk>(8000.f), ball_material)));
    /*prims.push_back(std::make_unique<TransformPrimitive>(
      light_ball_transform, std::make_unique<GeometricPrimitive>(
        light_ball, ball_material, ball_light)));*/
    prims.push_back(std::make_unique<TransformPrimitive>(
      light_disk_transform, std::make_unique<GeometricPrimitive>(
        light_disk, ball_material, disk_light)));

    Scene scene;
    /*scene.primitive = std::make_unique<BvhPrimitive>(
        std::move(prims), 2, BvhSplitMethod::Middle);*/
    scene.primitive = std::make_unique<ListPrimitive>(std::move(prims));
    /*scene.lights.push_back(std::make_shared<PointLight>(
          Point(-600.f, 0.f, -600.f), 4000000.f * Spectrum(0.f, 0.f, 1.f)));
    scene.lights.push_back(std::make_shared<PointLight>(
          Point(0.f, 0.f, -800.f), 1000000.f * Spectrum(0.f, 1.f, 0.f)));*/
    /*scene.lights.push_back(std::make_shared<PointLight>(
          Point(0.f, 200.f, 600.f), 1000000.f * Spectrum(1.f, 0.f, 0.f)));*/
    //scene.lights.push_back(ball_light);
    scene.lights.push_back(disk_light);

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

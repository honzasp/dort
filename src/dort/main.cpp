#include <cstdio>
#include <memory>
#include <vector>
#include "dort/basic_texture_maps.hpp"
#include "dort/basic_textures.hpp"
#include "dort/bvh_primitive.hpp"
#include "dort/diffuse_light.hpp"
#include "dort/direct_renderer.hpp"
#include "dort/disk.hpp"
#include "dort/lambertian_brdf.hpp"
#include "dort/list_primitive.hpp"
#include "dort/main.hpp"
#include "dort/matte_material.hpp"
#include "dort/plastic_material.hpp"
#include "dort/point_light.hpp"
#include "dort/primitive.hpp"
#include "dort/read_ply.hpp"
#include "dort/specular_materials.hpp"
#include "dort/sphere.hpp"
#include "dort/triangle_mesh.hpp"

namespace dort {
  int main() {
    auto ball_transform =
      translate(0.f, 0.f, 0.f) *
      rotate_x(-0.8f * PI) *
      rotate_z(-0.3f * PI);
    auto ball_material = std::make_shared<MatteMaterial>(
        checkerboard_texture(
          spherical_texture_map_2d(ball_transform),
          0.1f,
          Spectrum(1.f, 0.8f, 0.8f),
          Spectrum(0.8f, 1.f, 0.8f)),
        const_texture(1.5f));
    auto ball_prim = std::make_unique<GeometricPrimitive>(
        std::make_shared<Sphere>(200.f), ball_material);

    std::vector<std::unique_ptr<Primitive>> prims;
    prims.push_back(std::make_unique<TransformPrimitive>(
      ball_transform, std::move(ball_prim)));

    Scene scene;
    scene.primitive = std::make_unique<BvhPrimitive>(
        std::move(prims), 2, BvhSplitMethod::Middle);
    scene.lights.push_back(std::make_shared<PointLight>(
          Point(-200.f, 0.f, -800.f), 2e6f * Spectrum(1.f, 1.f, 1.f)));

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

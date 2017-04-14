#pragma once
#include <memory>
#include <vector>
#include "dort/dort.hpp"
#include "dort/box.hpp"

namespace dort {
  struct Scene {
    Box bounds;
    Point centroid;
    float radius;

    std::unique_ptr<Primitive> primitive;
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Light>> background_lights;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<PrimitiveMesh>> prim_meshes;
    std::shared_ptr<Camera> default_camera;

    bool intersect(Ray& ray, Intersection& out_isect) const;
    bool intersect_p(const Ray& ray) const;
  };

  DiscreteDistrib1d compute_light_distrib(const Scene& scene,
      bool only_background = false);
}

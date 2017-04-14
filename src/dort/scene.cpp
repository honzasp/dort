#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/primitive.hpp"
#include "dort/scene.hpp"
#include "dort/stats.hpp"

namespace dort {
  bool Scene::intersect(Ray& ray, Intersection& out_isect) const {
    stat_count(COUNTER_SCENE_INTERSECT);
    StatTimer t(TIMER_SCENE_INTERSECT);
    return this->primitive->intersect(ray, out_isect);
  }

  bool Scene::intersect_p(const Ray& ray) const {
    stat_count(COUNTER_SCENE_INTERSECT_P);
    StatTimer t(TIMER_SCENE_INTERSECT_P);
    return this->primitive->intersect_p(ray);
  }

  DiscreteDistrib1d compute_light_distrib(const Scene& scene, bool only_background) {
    const auto& lights = only_background ? scene.background_lights : scene.lights;
    std::vector<float> powers(lights.size());
    for(uint32_t i = 0; i < lights.size(); ++i) {
      const auto& light = lights.at(i);
      powers.at(i) = light->approximate_power(scene).average();
    }
    return DiscreteDistrib1d(powers);
  }
}


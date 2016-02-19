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
}


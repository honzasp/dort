#include "dort/light.hpp"
#include "dort/primitive.hpp"

namespace dort {
  void ShadowTest::init_point_point(const Point& p1, float epsilon_1,
      const Point& p2, float epsilon_2)
  {
    float distance = length(p2 - p1);
    this->ray.orig = p1;
    this->ray.dir = (p2 - p1) / distance;
    this->ray.t_min = epsilon_1;
    this->ray.t_max = distance - epsilon_2;
  }

  void ShadowTest::init_point_dir(const Point& pt, float epsilon,
      const Vector& dir)
  {
    this->ray.orig = pt;
    this->ray.dir = normalize(dir);
    this->ray.t_min = epsilon;
    this->ray.t_max = INFINITY;
  }

  bool ShadowTest::visible(const Scene& scene) const
  {
    return !scene.primitive->intersect_p(this->ray);
  }
  
  Spectrum Light::background_radiance(const Ray&) const
  {
    return Spectrum();
  }
}

#include "dort/light.hpp"
#include "dort/primitive.hpp"
#include "dort/whitted_renderer.hpp"

namespace dort {
  Spectrum WhittedRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth) const
  {
    Spectrum radiance(0.f);

    Intersection isect;
    if(!scene.primitive->intersect(ray, isect)) {
      for(const auto& light: scene.lights) {
        radiance = radiance + light->background_radiance(ray);
      }
      return radiance;
    }

    Point pt = isect.diff_geom.p;
    Normal n = isect.diff_geom.nn;

    Spectrum color = isect.primitive->get_color(isect.diff_geom);
    float reflection = isect.primitive->get_reflection(isect.diff_geom);

    for(const auto& light: scene.lights) {
      Vector wi;
      float pdf;
      ShadowTest shadow;
      Spectrum incident_radiance = light->sample_radiance(pt, isect.ray_epsilon,
          wi, pdf, shadow);
      if(incident_radiance.is_black() || pdf == 0.f) {
        continue;
      }

      if(shadow.visible(scene)) {
        radiance = radiance + (1.f - reflection) * color * incident_radiance 
          * abs_dot(wi, n) / pdf;
      }
    }

    if(depth < this->max_depth) {
      if(reflection != 0.f) {
        Vector wo = normalize(-ray.dir);
        Vector wi = 2.f * Vector(n.v) * dot(wo, n) - wo;
        Ray refl_ray(pt, wi, isect.ray_epsilon, INFINITY);
        Spectrum refl_radiance = this->get_radiance(scene, refl_ray, depth + 1);
        radiance = radiance + refl_radiance * reflection * color;
      }

      // TODO: refract
    }

    return radiance;
  }
}

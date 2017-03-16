#include "dort/bsdf.hpp"
#include "dort/dot_renderer.hpp"
#include "dort/primitive.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  Spectrum DotRenderer::get_radiance(const Scene& scene, Ray& ray, Vec2,
      uint32_t, Sampler&) const
  {
    Intersection isect;
    if(!scene.intersect(ray, isect)) {
      return Spectrum(0.f);
    }

    auto bsdf = isect.get_bsdf();
    float isect_dot = dot(-ray.dir, isect.world_diff_geom.nn);

    bool emittor = !isect.eval_radiance(ray.orig).is_black();
    bool specular = bsdf->num_bxdfs(BSDF_DELTA) > 0;
    bool backside = isect_dot < 0.f;

    float red = backside ? 0.8f : 0.5f;
    float green = emittor ? 0.8f : 0.5f;
    float blue = specular ? 0.8f : 0.5f;

    Spectrum color = Spectrum::from_rgb(red, green, blue);
    return color * abs(isect_dot);
  }

  void DotRenderer::preprocess(CtxG&, const Scene&, Sampler&) {
  }
}


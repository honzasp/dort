#include "dort/bsdf.hpp"
#include "dort/camera.hpp"
#include "dort/dot_renderer.hpp"
#include "dort/film.hpp"
#include "dort/primitive.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"
#include "dort/vec_2i.hpp"

namespace dort {
  void DotRenderer::render(CtxG& ctx, Progress&) {
    bool jitter = this->iteration_count > 1;
    for(uint32_t i = 0; i < this->iteration_count; ++i) {
      this->iteration_tiled(ctx, [&](Vec2i pixel, Vec2& film_pos, Sampler& sampler) {
        Vec2 pixel_pos = jitter ? sampler.random_2d() : Vec2(0.5f, 0.5f);
        film_pos = Vec2(pixel) + pixel_pos;

        Ray ray;
        float ray_pos_pdf;
        float ray_dir_pdf;
        Spectrum importance = this->camera->sample_ray_importance(Vec2(this->film->res),
          film_pos, ray, ray_pos_pdf, ray_dir_pdf, CameraSample(sampler.rng));
        Spectrum color = this->get_color(ray);
        return color * importance / (ray_pos_pdf * ray_dir_pdf);
      });
    }
  }

  Spectrum DotRenderer::get_color(Ray& ray) const {
    Intersection isect;
    if(!this->scene->intersect(ray, isect)) {
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
}


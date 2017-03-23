#include "dort/lighting.hpp"
#include "dort/path_renderer.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum PathRenderer::get_radiance(const Scene& scene, Ray& ray, Vec2,
      uint32_t, Sampler& sampler) const
  {
    Spectrum radiance(0.f);
    Spectrum throughput(1.f);
    Ray next_ray(ray);
    bool last_bxdf_was_specular = false;

    uint32_t bounces = 0;
    for(;;) {
      Intersection isect;
      bool isected = scene.intersect(next_ray, isect);

      if(bounces >= this->min_depth && (bounces == 0 || last_bxdf_was_specular)) {
        if(isected) {
          radiance += throughput * isect.eval_radiance(next_ray.orig);
        } else {
          for(const auto& light: scene.lights) {
            if(light->flags & LIGHT_BACKGROUND) {
              radiance += throughput * light->background_radiance(next_ray);
            }
          }
        }
      }

      if(!isected || bounces >= this->max_depth) {
        break;
      }
      ++bounces;

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo_camera = normalize(-next_ray.dir);
      geom.ray_epsilon = isect.ray_epsilon;
      auto bsdf = isect.get_bsdf();

      if(bounces >= this->min_depth) {
        Spectrum lighting;
        if(bounces < this->bsdf_samples_idxs.size()) {
          lighting = uniform_sample_one_light(scene, geom, *bsdf, sampler,
              sampler.get_sample_1d(this->light_idxs.at(bounces)),
              LightSample(sampler, this->light_samples_idxs.at(bounces), 0),
              BsdfSample(sampler, this->bsdf_samples_idxs.at(bounces), 0));
        } else {
          lighting = uniform_sample_one_light(scene, geom, *bsdf, sampler,
              sampler.random_1d(), LightSample(sampler.rng), BsdfSample(sampler.rng));
        }
        assert(is_finite(lighting) && is_nonnegative(lighting));
        radiance += lighting * throughput;
      }

      Vector bsdf_wi;
      float bsdf_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_light_f(geom.wo_camera, BSDF_ALL,
          bsdf_wi, bsdf_pdf, bsdf_flags,
          bounces < this->next_bsdf_samples_idxs.size()
            ? BsdfSample(sampler, this->next_bsdf_samples_idxs.at(bounces), 0)
            : BsdfSample(sampler.rng));

      if(bsdf_f.is_black() || bsdf_pdf == 0.f) {
        break;
      }
      throughput = throughput * bsdf_f * (abs_dot(bsdf_wi, geom.nn) / bsdf_pdf);
      assert(is_finite(throughput) && is_nonnegative(throughput));

      next_ray = Ray(geom.p, bsdf_wi, geom.ray_epsilon);
      last_bxdf_was_specular = bsdf_flags & BSDF_DELTA;

      if(bounces > this->max_depth / 2 && bounces > this->min_depth) {
        float term_prob = max(0.05f, 1.f - throughput.average());
        if(sampler.random_1d() < term_prob) {
          break;
        }
        throughput = throughput / term_prob;
      }
    }

    return radiance;
  }

  void PathRenderer::preprocess(CtxG&, const Scene&, Sampler& sampler) {
    for(uint32_t i = 0; i < this->max_depth && i < MAX_SAMPLES; ++i) {
      this->light_idxs.push_back(sampler.request_sample_1d());
      this->light_samples_idxs.push_back(LightSample::request(sampler, 1));
      this->bsdf_samples_idxs.push_back(BsdfSample::request(sampler, 1));
      this->next_bsdf_samples_idxs.push_back(BsdfSample::request(sampler, 1));
    }
  }
}

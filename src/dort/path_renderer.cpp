#include "dort/lighting.hpp"
#include "dort/path_renderer.hpp"
#include "dort/primitive.hpp"

namespace dort {
  Spectrum PathRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t, Sampler& sampler) const
  {
    Spectrum radiance(0.f);
    Spectrum throughput(1.f);
    Ray next_ray(ray);
    bool last_bxdf_was_specular = false;

    for(uint32_t bounces = 0; ; ++bounces) {
      Intersection isect;
      bool isected = scene.intersect(next_ray, isect);

      if(bounces == 0 || last_bxdf_was_specular) {
        if(isected) {
          radiance += isect.emitted_radiance(-next_ray.dir);
        } else {
          for(const auto& light: scene.lights) {
            if(light->flags & LIGHT_BACKGROUND) {
              radiance += light->background_radiance(next_ray);
            }
          }
        }
      }

      if(!isected || bounces >= this->max_depth) {
        break;
      }

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo = normalize(-next_ray.dir);
      geom.ray_epsilon = isect.ray_epsilon;
      auto bsdf = isect.get_bsdf();

      Spectrum lighting;
      if(bounces < this->bsdf_samples_idxs.size()) {
        lighting = uniform_sample_one_light(scene, geom, *bsdf, sampler,
            sampler.get_sample_1d(this->light_idxs.at(bounces)),
            LightSample(sampler, this->light_samples_idxs.at(bounces), 0),
            BsdfSample(sampler, this->bsdf_samples_idxs.at(bounces), 0));
      } else {
        lighting = uniform_sample_one_light(scene, geom, *bsdf, sampler,
            sampler.random_1d(), LightSample(sampler), BsdfSample(sampler));
      }
      assert(is_finite(lighting) && is_nonnegative(lighting));
      radiance += lighting * throughput;

      Vector bsdf_wi;
      float bsdf_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f;
      if(bounces < this->next_bsdf_samples_idxs.size()) {
        bsdf_f = bsdf->sample_f(geom.wo, bsdf_wi, bsdf_pdf, BSDF_ALL, bsdf_flags,
            BsdfSample(sampler, this->next_bsdf_samples_idxs.at(bounces), 0));
      } else {
        bsdf_f = bsdf->sample_f(geom.wo, bsdf_wi, bsdf_pdf, BSDF_ALL, bsdf_flags,
            BsdfSample(sampler));
      }

      if(bsdf_f.is_black() || bsdf_pdf == 0.f) {
        break;
      }
      throughput = throughput * bsdf_f * abs_dot(bsdf_wi, geom.nn) / bsdf_pdf;
      assert(is_finite(throughput) && is_nonnegative(throughput));

      next_ray = Ray(geom.p, bsdf_wi, geom.ray_epsilon);
      last_bxdf_was_specular = bsdf_flags & BSDF_SPECULAR;

      if(bounces > 3) {
        float term_prob = max(0.05f, 1.f - throughput.average());
        if(sampler.random_1d() < term_prob) {
          break;
        }
        throughput = throughput / term_prob;
      }
    }

    return radiance;
  }

  void PathRenderer::do_preprocess(const Scene&, Sampler& sampler) {
    for(uint32_t i = 0; i < this->max_depth && i < MAX_SAMPLES; ++i) {
      this->light_idxs.push_back(sampler.request_sample_1d());
      this->light_samples_idxs.push_back(LightSample::request(sampler, 1));
      this->bsdf_samples_idxs.push_back(BsdfSample::request(sampler, 1));
      this->next_bsdf_samples_idxs.push_back(BsdfSample::request(sampler, 1));
    }
  }
}

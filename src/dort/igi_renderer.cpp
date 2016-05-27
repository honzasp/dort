#include "dort/igi_renderer.hpp"
#include "dort/lighting.hpp"
#include "dort/low_discrepancy.hpp"
#include "dort/primitive.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  Spectrum IgiRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t depth, Sampler& sampler) const
  {
    Spectrum radiance(0.f);

    Intersection isect;
    if(!scene.intersect(ray, isect)) {
      for(const auto& light: scene.lights) {
        if(light->flags & LIGHT_BACKGROUND) {
          radiance += light->background_radiance(ray);
        }
      }
      return radiance;
    }

    Spectrum emitted = isect.emitted_radiance(-ray.dir);
    assert(is_finite(emitted) && is_nonnegative(emitted));
    radiance += emitted;

    LightingGeom geom;
    geom.p = isect.world_diff_geom.p;
    geom.nn = isect.world_diff_geom.nn;
    geom.wo = normalize(-ray.dir);
    geom.ray_epsilon = isect.ray_epsilon;

    std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
    radiance += uniform_sample_all_lights(scene, geom, *bsdf, sampler,
        make_slice(this->light_samples_idxs),
        make_slice(this->bsdf_samples_idxs));
    radiance += this->sample_virtual_lights(scene, geom, *bsdf, sampler);

    if(depth < this->max_depth) {
      Spectrum reflection = trace_specular(*this, scene,
          geom, *bsdf, BSDF_REFLECTION, depth, sampler);
      Spectrum transmission = trace_specular(*this, scene,
          geom, *bsdf, BSDF_TRANSMISSION, depth, sampler);
      assert(is_finite(reflection) && is_nonnegative(reflection));
      assert(is_finite(transmission) && is_nonnegative(transmission));
      radiance += reflection + transmission;
    }

    return radiance;
  }

  void IgiRenderer::do_preprocess(CtxG& ctx, const Scene& scene, Sampler& sampler) {
    this->light_sets = this->compute_light_sets(ctx, scene, sampler);

    this->light_set_idx = sampler.request_sample_1d();
    for(const auto& light: scene.lights) {
      uint32_t num_samples = max(1u, sampler.round_count(light->num_samples));
      this->light_samples_idxs.push_back(LightSample::request(sampler, num_samples));
      this->bsdf_samples_idxs.push_back(BsdfSample::request(sampler, num_samples));
    }
  }

  Spectrum IgiRenderer::sample_virtual_lights(const Scene& scene,
      const LightingGeom& isect_geom, const Bsdf& isect_bsdf, Sampler& sampler) const
  {
    uint32_t set_i = clamp(floor_uint32(
          sampler.get_sample_1d(this->light_set_idx) * float(this->light_sets.size())),
        0u, uint32_t(this->light_sets.size() - 1));
    const auto& light_set = this->light_sets.at(set_i);

    float weight = 1.f / float(light_set.size());
    Spectrum radiance;
    for(const VirtualLight& light: light_set) {
      Vector isect_wi = normalize(light.p - isect_geom.p);
      Spectrum isect_f = isect_bsdf.f(isect_geom.wo, isect_wi, BSDF_ALL);
      Spectrum light_f = light.bsdf->f(-isect_wi, light.wi, BSDF_ALL);
      float g = abs_dot(isect_wi, isect_geom.nn) * abs_dot(isect_wi, light.nn)
        / length_squared(isect_geom.p - light.p);
      g = min(g, this->g_limit);
      Spectrum light_contrib = isect_f * light_f * light.path_contrib * (weight * g);
      if(light_contrib.is_black()) {
        continue;
      }

      float light_contrib_avg = light_contrib.average();
      if(light_contrib_avg < this->roulette_threshold) {
        float survive_prob = 0.5f * light_contrib_avg / this->roulette_threshold;
        if(sampler.random_1d() > survive_prob) {
          continue;
        }
        light_contrib = light_contrib / survive_prob;
      }

      ShadowTest shadow;
      shadow.init_point_point(isect_geom.p, isect_geom.ray_epsilon,
          light.p, light.ray_epsilon);
      if(!shadow.visible(scene)) {
        continue;
      }

      radiance += light_contrib;
    }

    return radiance;
  }

  DiscreteDistrib1d IgiRenderer::compute_light_distrib(const Scene& scene) const {
    std::vector<float> powers(scene.lights.size());
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      const auto& light = scene.lights.at(i);
      float power = light->approximate_power(scene).average();
      float weight = float(light->num_samples);
      powers.at(i) = power * weight;
    }
    return DiscreteDistrib1d(powers);
  }

  std::vector<std::vector<IgiRenderer::VirtualLight>> 
  IgiRenderer::compute_light_sets(CtxG& ctx, const Scene& scene, Sampler& sampler) const
  {
    uint32_t set_count = this->light_set_count;
    uint32_t path_count = this->path_count;
    DiscreteDistrib1d light_distrib(this->compute_light_distrib(scene));
    std::vector<std::vector<VirtualLight>> sets(set_count);
    std::vector<std::mutex> sets_mutexes(set_count);

    uint32_t desired_jobs = 16 * ctx.pool->num_threads();
    uint32_t jobs_per_set = max(1u, desired_jobs / set_count);
    uint32_t job_count = set_count * jobs_per_set;

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(sampler.split());
    }

    std::vector<float> light_idx_samples(low_discrepancy_1d(
          set_count, path_count, sampler.rng));
    std::vector<Vec2> light_pos_samples(low_discrepancy_2d(
          set_count, path_count, sampler.rng));
    std::vector<Vec2> light_dir_samples(low_discrepancy_2d(
          set_count, path_count, sampler.rng));

    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      uint32_t set_i = job_i / jobs_per_set;
      uint32_t chunk_i = job_i % jobs_per_set;
      uint32_t chunk_begin = chunk_i * path_count / jobs_per_set;
      uint32_t chunk_end = (chunk_i + 1) * path_count / jobs_per_set;

      auto& sampler = *samplers.at(job_i);

      std::vector<VirtualLight> virtual_lights;
      for(uint32_t path_i = chunk_begin; path_i < chunk_end; ++path_i) {
        uint32_t i = set_i * path_count + path_i;
        uint32_t light_i = light_distrib.sample(light_idx_samples.at(i));
        float light_pdf = light_distrib.pdf(light_i);
        this->compute_light_path(scene,
          *scene.lights.at(light_i), light_pdf,
          LightRaySample(light_pos_samples.at(i), light_dir_samples.at(i)),
          virtual_lights, sampler);
      }

      std::unique_lock<std::mutex> sets_lock(sets_mutexes.at(set_i));
      for(auto& virt: virtual_lights) {
        sets.at(set_i).push_back(std::move(virt));
      }
    });

    return sets;
  }

  void IgiRenderer::compute_light_path(const Scene& scene,
      const Light& light, float light_pdf,
      const LightRaySample& light_sample,
      std::vector<VirtualLight>& virtual_lights,
      Sampler& sampler) const
  {
    if(light_pdf == 0.f) {
      return;
    }

    Ray ray;
    Normal prev_nn;
    float light_ray_pdf;
    Spectrum light_radiance = light.sample_ray_radiance(
        scene, ray, prev_nn, light_ray_pdf, light_sample);
    if(light_radiance.is_black() || light_ray_pdf == 0.f) {
      return;
    }

    Spectrum path_contrib = light_radiance / (light_ray_pdf * light_pdf);
    for(uint32_t depth = 0; depth < this->max_light_depth; ++depth) {
      Intersection isect;
      if(!scene.intersect(ray, isect)) {
        break;
      }
      auto bsdf = isect.get_bsdf();

      Vector bounce_wi;
      float bounce_pdf;
      BxdfFlags bounce_flags;
      Spectrum bounce_f = bsdf->sample_f(-ray.dir, bounce_wi, bounce_pdf,
          BSDF_ALL, bounce_flags, BsdfSample(sampler));
      if(bounce_f.is_black() || bounce_pdf == 0.f) {
        break;
      }

      Spectrum bounce_contrib = bounce_f 
        * (abs_dot(bounce_wi, isect.world_diff_geom.nn) / bounce_pdf);

      if(!(bounce_flags & BSDF_SPECULAR)) {
        VirtualLight virt;
        virt.p = isect.world_diff_geom.p;
        virt.nn = isect.world_diff_geom.nn;
        virt.wi = -ray.dir;
        virt.ray_epsilon = isect.ray_epsilon;
        virt.path_contrib = path_contrib;
        virt.bsdf = std::move(bsdf);
        virtual_lights.push_back(std::move(virt));

        float survive_prob = min(0.95f, bounce_contrib.average());
        if(sampler.random_1d() > survive_prob) {
          break;
        }
        path_contrib = path_contrib / survive_prob;
      }

      ray = Ray(isect.world_diff_geom.p, bounce_wi, isect.ray_epsilon);
      path_contrib = path_contrib * bounce_contrib;
    }
  }
}

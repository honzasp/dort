#include "dort/bsdf.hpp"
#include "dort/camera.hpp"
#include "dort/ctx.hpp"
#include "dort/film.hpp"
#include "dort/light_renderer.hpp"
#include "dort/primitive.hpp"
#include "dort/sampler.hpp"
#include "dort/scene.hpp"
#include "dort/stats.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  void LightRenderer::render(CtxG& ctx, Progress& progress) {
    StatTimer t(TIMER_RENDER);

    uint64_t path_count = uint64_t(this->iteration_count) *
      this->film->res.x * this->film->res.y;
    uint32_t job_count = ctx.pool->thread_count() * 16;

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }
    this->light_distrib = compute_light_distrib(*this->scene);

    std::mutex film_mutex;
    std::atomic<uint32_t> jobs_done(0);
    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      if(progress.is_cancelled()) { return; }
      uint64_t begin = uint64_t(job_i) * path_count / job_count;
      uint64_t end = uint64_t(job_i + 1) * path_count / job_count;
      auto& sampler = *samplers.at(job_i);

      for(uint64_t i = begin; i < end; ++i) {
        this->sample_path(sampler);
      }

      uint32_t done = jobs_done.fetch_add(1) + 1;
      progress.set_percent_done(float(done) / float(job_count));
      {
        std::unique_lock<std::mutex> film_lock(film_mutex);
        this->film->splat_scale = float(job_count) / (float(done) 
          * float(this->iteration_count));
      }
    });

    this->film->splat_scale = 1.f / float(this->iteration_count);
  }

  void LightRenderer::sample_path(Sampler& sampler) {
    uint32_t light_i = this->light_distrib.sample(sampler.random_1d());
    float light_pick_pdf = this->light_distrib.pdf(light_i);
    if(light_pick_pdf == 0.f) { return; }

    const Light& light = *this->scene->lights.at(light_i);

    if(this->min_length <= 2 && 2 <= this->max_length) {
      // connect a point on light directly to camera
      this->sample_immediate(light, light_pick_pdf, sampler);
    }

    if(this->max_length < 3) { return; }

    Ray ray;
    Normal prev_nn;
    float light_pos_pdf, light_dir_pdf;
    Spectrum light_radiance = light.sample_ray_radiance(
        *this->scene, ray, prev_nn, light_pos_pdf, light_dir_pdf,
        LightRaySample(sampler.rng));

    float light_pdf = light_pick_pdf * light_pos_pdf * light_dir_pdf;
    if(light_radiance.is_black() || light_pdf == 0.f) { return; }

    Spectrum throughput = light_radiance / (light_pos_pdf * light_pick_pdf);
    float prev_dir_pdf = light_dir_pdf;
    for(uint32_t bounces = 0; bounces + 3 <= this->max_length; ++bounces) {
      Intersection isect;
      if(prev_dir_pdf == 0.f || !this->scene->intersect(ray, isect)) {
        break;
      }
      auto bsdf = isect.get_bsdf();

      Vector wi = normalize(-ray.dir);
      float geom = abs_dot(prev_nn, wi) / prev_dir_pdf;

      if(bounces + 3 >= this->min_length) {
        // connect the intersection to the camera
        this->connect_to_camera(isect, *bsdf, wi, throughput * geom, sampler);
      }

      Vector bounce_wo;
      float bounce_dir_pdf;
      BxdfFlags bounce_flags;
      Spectrum bounce_f = bsdf->sample_camera_f(-ray.dir, BSDF_ALL,
        bounce_wo, bounce_dir_pdf, bounce_flags, BsdfSample(sampler.rng));

      Spectrum bounce_contrib = bounce_f * geom;
      throughput *= bounce_contrib * geom;
      if(throughput.is_black()) { break; }
      assert(is_finite(throughput) && is_nonnegative(throughput));

      if(bounces >= 2) {
        float survive_prob = clamp(bounce_contrib.average(), 0.1f, 0.9f);
        if(sampler.random_1d() > survive_prob) { break; }
        throughput /= survive_prob;
      }

      prev_nn = isect.world_diff_geom.nn;
      prev_dir_pdf = bounce_dir_pdf;
      ray = Ray(isect.world_diff_geom.p, bounce_wo, isect.ray_epsilon);
    }
  }

  void LightRenderer::sample_immediate(const Light& light,
      float light_pick_pdf, Sampler& sampler)
  {
    Vec2 film_res(this->film->res);

    Point light_p;
    float light_epsilon;
    Normal light_nn;
    float light_pos_pdf;
    bool sampled = light.sample_point(light_p, light_epsilon,
        light_nn, light_pos_pdf, LightSample(sampler.rng)); 
    if(!sampled || light_pos_pdf == 0.f) { return; }

    float camera_pos_pdf;
    Point camera_p = this->camera->sample_point(film_res,
        camera_pos_pdf, CameraSample(sampler.rng));
    if(camera_pos_pdf == 0.f) { return; }

    Vec2 film_pos;
    Spectrum importance = this->camera->eval_importance(film_res,
        camera_p, normalize(light_p - camera_p), film_pos);
    if(importance.is_black()) { return; }
    Spectrum radiance = light.eval_radiance(light_p, light_nn, camera_p);
    if(radiance.is_black()) { return; }

    float contrib_pdf = light_pick_pdf * light_pos_pdf * camera_pos_pdf;
    if(contrib_pdf == 0.f) { return; }
    Spectrum contrib = radiance * importance /
      (length_squared(light_p - camera_p) * contrib_pdf);
    if(!(light_nn == Normal())) {
      contrib *= abs_dot(light_nn, normalize(light_p - camera_p));
    }
    if(contrib.is_black()) { return; }

    ShadowTest shadow;
    shadow.init_point_point(light_p, light_epsilon, camera_p, 0.f);
    if(shadow.visible(*this->scene)) {
      this->film->add_splat(film_pos, contrib);
    }
  }

  void LightRenderer::connect_to_camera(const Intersection& isect, const Bsdf& bsdf,
      const Vector& wi, const Spectrum& throughput, Sampler& sampler)
  {
    if(bsdf.bxdf_count(BSDF_ALL & (~BSDF_DELTA)) == 0) { return; }
    Vec2 film_res(this->film->res);

    Point camera_p;
    float camera_p_pdf;
    ShadowTest shadow;
    Vec2 film_pos;
    Spectrum importance = this->camera->sample_pivot_importance(film_res,
        isect.world_diff_geom.p, isect.ray_epsilon,
        camera_p, film_pos, camera_p_pdf,
        shadow, CameraSample(sampler.rng));
    if(camera_p_pdf == 0.f || importance.is_black()) { return; }

    Vector camera_wo = normalize(camera_p - isect.world_diff_geom.p);
    Spectrum bsdf_f = bsdf.eval_f(-wi, camera_wo, BSDF_ALL);
    if(bsdf_f.is_black()) { return; }

    Spectrum contrib = throughput * importance * bsdf_f *
        ( abs_dot(isect.world_diff_geom.nn, camera_wo)
        / (length_squared(camera_p - isect.world_diff_geom.p) * camera_p_pdf));
    assert(is_finite(contrib) && is_nonnegative(contrib));
    if(!contrib.is_black() && shadow.visible(*this->scene)) {
      this->film->add_splat(film_pos, contrib);
    }
  }
}

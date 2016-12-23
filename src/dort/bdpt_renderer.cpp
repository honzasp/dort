#include "dort/bdpt_renderer.hpp"
#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/lighting.hpp"
#include "dort/primitive.hpp"

namespace dort {
  // TODO: use sampling patterns
  // TODO: use optimized direct lighting when appropriate
  // TODO: connect light vertices directly to camera vertex (requires direct
  // Renderer subclass)
  // TODO: sample BSDFs from both directions correctly
  // TODO: optimize path pdfs and contribution computation
  // TODO: correctly handle delta distributions in BSDFs
  // TODO: add contributions from background lights
  Spectrum BdptRenderer::get_radiance(const Scene& scene, Ray& ray,
      uint32_t, Sampler& sampler) const
  {
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    const Camera& camera = *scene.camera;
    const Light* light;
    std::vector<Vertex> light_walk = this->random_light_walk(scene, light, sampler.rng);
    std::vector<Vertex> camera_walk = this->random_camera_walk(scene, ray, sampler.rng);

    //std::printf("%lu %lu\n", light_walk.size(), camera_walk.size());
    Spectrum contrib(0.f);
    for(uint32_t s = 1; s <= light_walk.size(); ++s) {
      for(uint32_t t = 1; t <= camera_walk.size(); ++t) {
        Vec2 film_pos;
        Spectrum path_contrib = this->path_contrib(scene, film_res, *light, camera,
            sampler.rng, light_walk, camera_walk, s, t, film_pos);
        if(path_contrib.is_black()) { continue; }

        float path_weight = this->path_weight(scene, film_res, film_pos, *light, camera,
            light_walk, camera_walk, s, t);
        Spectrum weighted_contrib = path_contrib * path_weight;
        //std::printf("  %d,%d: %g %g\n", s, t, path_contrib.average(), path_weight);
        if(t == 1) {
          // TODO: splat the path_value to the film
        } else {
          contrib += weighted_contrib;
        }
      }
    }

    //std::printf("  = %g\n", contrib.average());
    return contrib;
  }

  void BdptRenderer::preprocess(CtxG&, const Scene& scene, Sampler&) {
    this->light_distrib = compute_light_distrib(scene);
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      this->light_distrib_pdfs.insert(std::make_pair(
          scene.lights.at(i).get(), this->light_distrib.pdf(i)));
    }
  }

  std::vector<BdptRenderer::Vertex> BdptRenderer::random_light_walk(
      const Scene& scene, const Light*& out_light, Rng& rng) const 
  {
    uint32_t light_i = this->light_distrib.sample(rng.uniform_float());
    float light_pick_pdf = this->light_distrib.pdf(light_i);
    const Light& light = *scene.lights.at(light_i);
    out_light = &light;

    Ray light_ray;
    Normal light_nn;
    float light_pos_pdf, light_dir_pdf;
    Spectrum light_radiance = light.sample_ray_radiance(scene, light_ray, light_nn,
        light_pos_pdf, light_dir_pdf, LightRaySample(rng));

    std::vector<Vertex> walk;

    Vertex y0;
    y0.p = light_ray.orig;
    y0.p_epsilon = light_ray.t_min;
    y0.nn = light_nn;
    y0.bsdf = nullptr;
    y0.area_light = nullptr;
    y0.fwd_area_pdf = light_pos_pdf * light_pick_pdf;
    y0.bwd_area_pdf = SIGNALING_NAN;
    y0.alpha = light_radiance / (light_pos_pdf * light_pick_pdf);
    walk.push_back(std::move(y0));

    Spectrum prev_bsdf_f(1.f);
    float fwd_dir_pdf = light_dir_pdf;
    for(uint32_t bounces = 0; bounces < this->max_depth; ++bounces) {
      Vertex& prev_y = walk.at(walk.size() - 1);
      Intersection isect;
      if(!scene.intersect(light_ray, isect)) {
        break;
      }

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      Vector wi = -light_ray.dir;
      Vector wo;
      float wo_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_camera_f(wi, BSDF_ALL & (~BSDF_DELTA),
          wo, wo_pdf, bsdf_flags, BsdfSample(rng));

      float bwd_dir_pdf = bsdf->light_f_pdf(wi, wo, BSDF_ALL & (~BSDF_DELTA));
      float dist_squared = length_squared(isect.world_diff_geom.p - prev_y.p);
      if(dist_squared < 1e-9f) { break; }

      Vertex y;
      y.p = isect.world_diff_geom.p;
      y.p_epsilon = isect.ray_epsilon;
      y.nn = isect.world_diff_geom.nn;
      y.bsdf = std::move(bsdf);
      y.area_light = nullptr;
      y.fwd_area_pdf = fwd_dir_pdf * abs_dot(y.nn, wi) / dist_squared;
      y.bwd_area_pdf = SIGNALING_NAN;
      y.alpha = prev_y.alpha * prev_bsdf_f * (abs_dot(prev_y.nn, wi) / fwd_dir_pdf);
      if(y.alpha.is_black()) {
        break;
      }

      prev_y.bwd_area_pdf = bwd_dir_pdf * abs_dot(prev_y.nn, wi) / dist_squared;
      prev_bsdf_f = bsdf_f;
      fwd_dir_pdf = wo_pdf;
      light_ray = Ray(y.p, wo, isect.ray_epsilon);
      walk.push_back(std::move(y));
    }
    return walk;
  }

  std::vector<BdptRenderer::Vertex> BdptRenderer::random_camera_walk(
      const Scene& scene, const Ray& ray, Rng& rng) const
  {
    Ray camera_ray(ray);
    std::vector<Vertex> walk;

    Vertex z0;
    z0.p = camera_ray.orig;
    z0.p_epsilon = 0.f;
    z0.nn = Normal(camera_ray.dir);
    z0.bsdf = nullptr;
    z0.area_light = nullptr;
    z0.fwd_area_pdf = 1.f;
    z0.bwd_area_pdf = SIGNALING_NAN;
    z0.alpha = Spectrum(1.f);
    walk.push_back(std::move(z0));

    Spectrum prev_bsdf_f(1.f);
    float fwd_dir_pdf = 1.f;
    for(uint32_t bounces = 0; bounces < this->max_depth; ++bounces) {
      Vertex& prev_z = walk.at(walk.size() - 1);
      Intersection isect;
      if(!scene.intersect(camera_ray, isect)) {
        break;
      }

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      Vector wo = -camera_ray.dir;
      Vector wi;
      float wi_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_light_f(wo, BSDF_ALL & (~BSDF_DELTA),
          wi, wi_pdf, bsdf_flags, BsdfSample(rng));

      float bwd_dir_pdf = bsdf->camera_f_pdf(wo, wi, BSDF_ALL & (~BSDF_DELTA));
      float dist_squared = length_squared(isect.world_diff_geom.p - prev_z.p);
      if(dist_squared < 1e-9f) { break; }

      Vertex z;
      z.p = isect.world_diff_geom.p;
      z.p_epsilon = isect.ray_epsilon;
      z.nn = isect.world_diff_geom.nn;
      z.bsdf = std::move(bsdf);
      z.area_light = isect.get_area_light();
      z.fwd_area_pdf = fwd_dir_pdf * abs_dot(z.nn, wo) / dist_squared;;
      z.bwd_area_pdf = SIGNALING_NAN;
      z.alpha = prev_z.alpha * prev_bsdf_f * (abs_dot(prev_z.nn, wo) / fwd_dir_pdf);
      if(z.alpha.is_black()) {
        break;
      }

      prev_z.bwd_area_pdf = bwd_dir_pdf * abs_dot(prev_z.nn, wo) / dist_squared;
      prev_bsdf_f = bsdf_f;
      fwd_dir_pdf = wi_pdf;
      camera_ray = Ray(z.p, wi, isect.ray_epsilon);
      walk.push_back(std::move(z));
    }
    return walk;
  }

  Spectrum BdptRenderer::path_contrib(const Scene& scene, Vec2 film_res,
      const Light&, const Camera& camera, Rng& rng,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t, Vec2& out_film_pos) const
  {
    const Vertex& last_light = light_walk.at(s - 1);
    const Vertex& last_camera = camera_walk.at(t - 1);

    if(s == 1 && t == 1) {
      // Connect the camera vertex to a new light vertex.

      // There are two strategies possible:
      // a) pick a point y on light (with area pdf) and sample a direction to
      // point z on the camera (with solid angle pdf from y)
      // b) pick a point z on camera (with area pdf) and sample a direction to
      // point y on the light (with solid angle pdf from z)
      //
      // Note that a) cannot sample paths from delta-direction lights, while
      // b) cannot sample paths from delta-direction cameras. We could try both
      // methods and if both are applicable combine them with MIS, but this is
      // probably not worth the trouble.
      //
      // TODO: implement this
      out_film_pos = Vec2();
      return Spectrum(0.f);
    } else if(s == 0 && t >= 2) {
      // Use only the camera path, provided that the last vertex is emissive.
      if(last_camera.area_light == nullptr) { return Spectrum(0.f); }
      Vector wo = normalize(camera_walk.at(t - 2).p - last_camera.p);
      Spectrum emitted_radiance = last_camera.area_light->emitted_radiance(
          last_camera.p, last_camera.nn, wo);
      if(emitted_radiance.is_black()) { return Spectrum(0.f); }

      return emitted_radiance * last_camera.alpha;
    } else if(s == 1) {
      // Connect the camera subpath to a new light vertex.
      uint32_t light_i = this->light_distrib.sample(rng.uniform_float());
      float light_pick_pdf = this->light_distrib.pdf(light_i);
      const Light& light = *scene.lights.at(light_i);
      if(light_pick_pdf == 0.f) { return Spectrum(0.f); }

      Vector wi;
      float wi_dir_pdf;
      ShadowTest shadow;
      Spectrum light_radiance = light.sample_pivot_radiance(
          last_camera.p, last_camera.p_epsilon, wi, wi_dir_pdf, shadow,
          LightSample(rng));
      if(light_radiance.is_black() || wi_dir_pdf == 0.f) {
        return Spectrum(0.f);
      }
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector bsdf_wo = normalize(camera_walk.at(t - 2).p - last_camera.p);
      Spectrum bsdf_f = last_camera.bsdf->eval_f(wi, bsdf_wo, BSDF_ALL);
      if(bsdf_f.is_black()) { return Spectrum(0.f); }

      return last_camera.alpha * bsdf_f * light_radiance
        * (abs_dot(last_camera.nn, wi) / (light_pick_pdf * wi_dir_pdf));
    } else if(t == 1) {
      // Connect the light subpath to a new camera vertex.
      Vector wo;
      float wo_dir_pdf;
      ShadowTest shadow;
      Spectrum camera_importance = camera.sample_pivot_importance(film_res,
          last_light.p, last_light.p_epsilon, wo, wo_dir_pdf,
          shadow, out_film_pos, CameraSample(rng));
      if(camera_importance.is_black() || wo_dir_pdf == 0.f) {
        return Spectrum(0.f);
      }
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector bsdf_wi = normalize(light_walk.at(s - 2).p - last_light.p);
      Spectrum bsdf_f = last_light.bsdf->eval_f(bsdf_wi, wo, BSDF_ALL);
      if(bsdf_f.is_black()) { return Spectrum(0.f); }

      return last_light.alpha * bsdf_f * camera_importance
        * (abs_dot(last_light.nn, wo) / wo_dir_pdf);
    } else if(s >= 2 && t >= 2) {
      // Connect the light and camera subpaths.
      ShadowTest shadow;
      shadow.init_point_point(last_light.p, last_light.p_epsilon,
          last_camera.p, last_camera.p_epsilon);
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector light_wi = normalize(light_walk.at(s - 2).p - last_light.p);
      Vector camera_wo = normalize(camera_walk.at(t - 2).p - last_camera.p);
      Vector wo = normalize(last_camera.p - last_light.p);

      Spectrum light_bsdf_f = last_light.bsdf->eval_f(light_wi, wo, BSDF_ALL);
      if(light_bsdf_f.is_black()) { return Spectrum(0.f); }
      Spectrum camera_bsdf_f = last_camera.bsdf->eval_f(-wo, camera_wo, BSDF_ALL);
      if(camera_bsdf_f.is_black()) { return Spectrum(0.f); }

      float geom = abs_dot(wo, last_light.nn) * abs_dot(wo, last_camera.nn)
        / length_squared(last_light.p - last_camera.p);

      return last_light.alpha * last_camera.alpha * light_bsdf_f * camera_bsdf_f * geom;
    }
    assert(false && "Unhandled combination of path lengths");
    return Spectrum();
  }

  float BdptRenderer::path_weight(const Scene& scene, Vec2 film_res, Vec2 film_pos,
      const Light& light, const Camera& camera,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t) const
  {
    if(s + t <= 2) { return 1.f; }
    const Vertex& last_light = light_walk.at(s - 1);
    const Vertex& last_camera = camera_walk.at(t - 1);
    Vector connect_wi = normalize(last_light.p - last_camera.p);
    float dist_squared = length_squared(last_light.p - last_camera.p);

    // Compute the area pdf of sampling the last light vertex from the last
    // camera vertex.
    float last_light_bwd_dir_pdf = 0.f;
    if(t >= 2) {
      Vector camera_wo = normalize(camera_walk.at(t - 2).p - last_camera.p);
      last_light_bwd_dir_pdf = last_camera.bsdf->light_f_pdf(
          connect_wi, camera_wo, BSDF_ALL & (~BSDF_DELTA));
    } else if(t == 1) {
      last_light_bwd_dir_pdf = camera.ray_dir_importance_pdf(film_res, film_pos,
          connect_wi, last_camera.p);
    } else {
      last_light_bwd_dir_pdf = 0.f;
    }
    float last_light_bwd_pdf = last_light_bwd_dir_pdf 
      * abs_dot(last_light.nn, connect_wi) / dist_squared;

    // Compute the area pdf of sampling the last camera vertex from the last
    // light vertex.
    float last_camera_bwd_dir_pdf = 0.f;
    if(s >= 2) {
      Vector light_wi = normalize(light_walk.at(s - 2).p - last_light.p);
      last_camera_bwd_dir_pdf = last_light.bsdf->camera_f_pdf(
          -connect_wi, light_wi, BSDF_ALL & (~BSDF_DELTA));
    } else if(s == 1) {
      // TODO: this is probably not correct, because we sample a new light for
      // this case in path_contrib(), so the pdf is different. However, I guess
      // that the estimator may still be correct.
      // TODO: handle infinite lights
      last_camera_bwd_dir_pdf = light.ray_dir_radiance_pdf(
          scene, -connect_wi, last_light.p, last_light.nn);
    } else if(s == 0) {
      float light_pick_pdf = this->light_distrib_pdfs.at(&light);
      last_camera_bwd_dir_pdf = light_pick_pdf * light.ray_origin_radiance_pdf(
          scene, last_camera.p, -connect_wi);
    }
    float last_camera_bwd_pdf = last_camera_bwd_dir_pdf 
      * abs_dot(last_camera.nn, connect_wi) / dist_squared;

    // Sum all the relative probabilities to obtain the MIS weight.
    float inv_weight_sum = 1.f;

    float ri_light = 1.f;
    for(uint32_t i = s; i-- >= 1;) {
      const Vertex& yi = light_walk.at(i);
      float yi_bwd_pdf = i < s - 1 ? yi.bwd_area_pdf : last_light_bwd_pdf;
      assert(yi.fwd_area_pdf > 0.f); assert(is_finite(yi_bwd_pdf));
      ri_light = ri_light * yi_bwd_pdf / yi.fwd_area_pdf;
      inv_weight_sum += ri_light;
    }

    float ri_camera = 1.f;
    for(uint32_t i = t; i-- >= 2;) {
      const Vertex& zi = camera_walk.at(i);
      float zi_bwd_pdf = i < t - 1 ? zi.bwd_area_pdf : last_camera_bwd_pdf;;
      assert(zi.fwd_area_pdf > 0.f); assert(is_finite(zi_bwd_pdf));
      ri_camera = ri_camera * zi_bwd_pdf / zi.fwd_area_pdf;
      // TODO: paths with t == 1 are ignored now
      if(i == 1) { continue; }
      inv_weight_sum += ri_camera;
    }

    assert(is_finite(inv_weight_sum));
    return 1.f / inv_weight_sum;
  }
}

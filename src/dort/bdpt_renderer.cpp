#include "dort/bdpt_renderer.hpp"
#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/lighting.hpp"
#include "dort/primitive.hpp"

namespace dort {
  // TODO: use sampling patterns
  // TODO: use optimized direct lighting when appropriate
  // TODO: correctly handle delta distributions in BSDFs
  // TODO: correctly handle background (infinite area) lights
  // TODO: implement the s==1 && t==1 case
  Spectrum BdptRenderer::get_radiance(const Scene& scene, Ray& ray, Vec2 film_pos,
      uint32_t, Sampler& sampler) const
  {
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    const Camera& camera = *this->camera;
    const Light* light;
    std::vector<Vertex> light_walk = this->random_light_walk(scene, light, sampler.rng);
    std::vector<Vertex> camera_walk = this->random_camera_walk(scene, ray, sampler.rng);

    Spectrum contrib(0.f);
    for(uint32_t s = 0; s <= light_walk.size(); ++s) {
      for(uint32_t t = 1; t <= camera_walk.size(); ++t) {
        if(t == 1 && !this->use_t1_paths) { continue; }
        if(s + t < this->min_depth + 2) { continue; }
        if(s + t > this->max_depth + 2) { continue; }

        Vec2 new_film_pos;
        Spectrum path_contrib = this->path_contrib(scene, film_res, *light, camera,
            sampler.rng, light_walk, camera_walk, s, t, new_film_pos);
        if(path_contrib.is_black()) { continue; }

        Vec2 path_film_pos = t == 1 ? new_film_pos : film_pos;
        float path_weight = this->path_weight(scene, film_res,
            *light, camera, path_film_pos,
            light_walk, camera_walk, s, t);
        Spectrum weighted_contrib = path_contrib * path_weight;

        if(t == 1) {
          this->film->add_splat(path_film_pos, weighted_contrib);
        } else {
          contrib += weighted_contrib;
        }

        if(!this->debug_image_dir.empty()) {
          this->store_debug_contrib(s, t, false, path_film_pos, path_contrib);
          this->store_debug_contrib(s, t, true, path_film_pos, weighted_contrib);
        }
      }
    }

    return contrib;
  }

  void BdptRenderer::preprocess(CtxG&, const Scene& scene, Sampler&) {
    this->light_distrib = compute_light_distrib(scene);
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      this->light_distrib_pdfs.insert(std::make_pair(
          scene.lights.at(i).get(), this->light_distrib.pdf(i)));
    }

    if(!this->debug_image_dir.empty()) {
      this->init_debug_films();
    }
  }

  void BdptRenderer::postprocess(CtxG&) {
    if(!this->debug_image_dir.empty()) {
      this->save_debug_films();
    }
  }

  void BdptRenderer::iteration(Film& film, uint32_t iteration) {
    uint64_t splat_weight = uint64_t(iteration + 1) 
      * this->sampler->samples_per_pixel;
    film.splat_scale = 1.f / float(splat_weight);
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
    y0.fwd_area_pdf = light_pos_pdf;
    y0.bwd_area_pdf = SIGNALING_NAN;
    y0.alpha = light_radiance / (light_pos_pdf * light_pick_pdf);
    y0.is_delta = false;
    walk.push_back(std::move(y0));

    Spectrum prev_bsdf_f(1.f);
    float fwd_dir_pdf = light_dir_pdf;
    for(uint32_t bounces = 0; bounces <= this->max_depth; ++bounces) {
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
      Spectrum bsdf_f = bsdf->sample_camera_f(wi, BSDF_ALL,
          wo, wo_pdf, bsdf_flags, BsdfSample(rng));
      if(wo_pdf == 0.f) { break; }

      float bwd_dir_pdf = bsdf->light_f_pdf(wi, wo, BSDF_ALL);
      float dist_squared = length_squared(isect.world_diff_geom.p - prev_y.p);
      if(dist_squared < 1e-9f) { break; }

      Spectrum alpha_scale = prev_bsdf_f * (abs_dot(prev_y.nn, wi) / fwd_dir_pdf);
      if(bounces >= 2) {
        float rr_prob = min(alpha_scale.average(), 0.9f);
        if(rng.uniform_float() > rr_prob) { break; }
        alpha_scale /= rr_prob;
      }

      Vertex y;
      y.p = isect.world_diff_geom.p;
      y.p_epsilon = isect.ray_epsilon;
      y.nn = isect.world_diff_geom.nn;
      y.bsdf = std::move(bsdf);
      y.area_light = nullptr;
      y.fwd_area_pdf = prev_y.is_delta ? fwd_dir_pdf
        : fwd_dir_pdf * abs_dot(y.nn, wi) / dist_squared;
      y.bwd_area_pdf = SIGNALING_NAN;
      y.alpha = prev_y.alpha * alpha_scale;
      y.is_delta = bsdf_flags & BSDF_DELTA;
      if(y.alpha.is_black()) {
        break;
      }

      prev_y.bwd_area_pdf = y.is_delta ? wo_pdf
        : bwd_dir_pdf * abs_dot(prev_y.nn, wi) / dist_squared;
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
    for(uint32_t bounces = 0; bounces <= this->max_depth; ++bounces) {
      Vertex& prev_z = walk.at(walk.size() - 1);
      if(prev_z.alpha.is_black()) { break; }

      Intersection isect;
      if(!scene.intersect(camera_ray, isect)) { break; }

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      Vector wo = -camera_ray.dir;
      Vector wi;
      float wi_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = bsdf->sample_light_f(wo, BSDF_ALL,
          wi, wi_pdf, bsdf_flags, BsdfSample(rng));
      if(wi_pdf == 0.f) { break; }

      float bwd_dir_pdf = bsdf->camera_f_pdf(wo, wi, BSDF_ALL);
      float dist_squared = length_squared(isect.world_diff_geom.p - prev_z.p);
      if(dist_squared < 1e-9f) { break; }

      Spectrum alpha_scale = prev_bsdf_f * (abs_dot(prev_z.nn, wo) / fwd_dir_pdf);
      if(bounces >= 2) {
        float rr_prob = min(alpha_scale.average(), 0.9f);
        if(rng.uniform_float() > rr_prob) { break; }
        alpha_scale /= rr_prob;
      }

      Vertex z;
      z.p = isect.world_diff_geom.p;
      z.p_epsilon = isect.ray_epsilon;
      z.nn = isect.world_diff_geom.nn;
      z.bsdf = std::move(bsdf);
      z.area_light = isect.get_area_light();
      z.fwd_area_pdf = prev_z.is_delta ? fwd_dir_pdf
        : fwd_dir_pdf * abs_dot(z.nn, wo) / dist_squared;;
      z.bwd_area_pdf = SIGNALING_NAN;
      z.alpha = prev_z.alpha * alpha_scale;
      z.is_delta = bsdf_flags & BSDF_DELTA;
      if(z.alpha.is_black() && !z.area_light) {
        break;
      }

      prev_z.bwd_area_pdf = z.is_delta ? wi_pdf
        : bwd_dir_pdf * abs_dot(prev_z.nn, wo) / dist_squared;
      prev_bsdf_f = bsdf_f;
      fwd_dir_pdf = wi_pdf;
      camera_ray = Ray(z.p, wi, isect.ray_epsilon);
      walk.push_back(std::move(z));
    }
    return walk;
  }

  Spectrum BdptRenderer::path_contrib(const Scene& scene, Vec2 film_res,
      const Light& light, const Camera& camera, Rng& rng,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t, Vec2& out_film_pos) const
  {
    const Vertex& last_camera = camera_walk.at(t - 1);

    if(s == 1 && t == 1) {
      // Connect the camera vertex to a new light vertex.
      if(!(this->camera->flags & CAMERA_DIR_BY_POS_DELTA)) {
        // pick a point z on camera, sample light direction from this point,
        // sample film position from this direction
        float camera_epsilon;
        float camera_p_pdf;
        Point camera_p = camera.sample_point(camera_epsilon,
            camera_p_pdf, CameraSample(rng));
        if(camera_p_pdf == 0.f) { return Spectrum(0.f); }

        Vector wi;
        Normal light_nn;
        float wi_dir_pdf;
        ShadowTest shadow;
        Spectrum radiance = light.sample_pivot_radiance(camera_p, camera_epsilon,
            wi, wi_dir_pdf, shadow, LightSample(rng));
        if(wi_dir_pdf == 0.f || radiance.is_black()) { return Spectrum(0.f); }

        float film_pdf;
        Spectrum importance = camera.sample_film_pos(film_res, camera_p, wi,
            out_film_pos, film_pdf);
        if(film_pdf == 0.f || importance.is_black()) { return Spectrum(0.f); }

        float light_pdf = this->light_distrib_pdfs.at(&light);
        if(light_pdf == 0.f) { return Spectrum(0.f); }
        if(!shadow.visible(scene)) { return Spectrum(0.f); }

        return (importance * radiance) 
          / (camera_p_pdf * wi_dir_pdf * film_pdf * light_pdf);
      } else {
        // TODO: pick a point y on light, sample camera point, direction and
        // film position to look at this point, evaluate radiance from the light
        // in the direction
        out_film_pos = Vec2();
        return Spectrum(0.f);
      }
    } else if(s == 0 && t >= 2) {
      // Use only the camera path, provided that the last vertex is emissive.
      if(last_camera.area_light == nullptr) { return Spectrum(0.f); }
      Spectrum emitted_radiance = last_camera.area_light->eval_radiance(
          last_camera.p, last_camera.nn, camera_walk.at(t - 2).p);
      if(emitted_radiance.is_black()) { return Spectrum(0.f); }

      return emitted_radiance * last_camera.alpha;
    } else if(s == 1) {
      /*
      // Connect the camera subpath to a new light vertex.
      uint32_t light_i = this->light_distrib.sample(rng.uniform_float());
      float light_pick_pdf = this->light_distrib.pdf(light_i);
      const Light& light = *scene.lights.at(light_i);
      */
      float light_pick_pdf = this->light_distrib_pdfs.at(&light);
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
      const Vertex& last_light = light_walk.at(s - 1);
      Vector wo;
      float wo_dir_pdf;
      float film_pdf;
      ShadowTest shadow;
      Spectrum camera_importance = camera.sample_pivot_importance(film_res,
          last_light.p, last_light.p_epsilon,
          wo, out_film_pos, wo_dir_pdf, film_pdf,
          shadow, CameraSample(rng));
      if(camera_importance.is_black() || wo_dir_pdf == 0.f) {
        return Spectrum(0.f);
      }
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector bsdf_wi = normalize(light_walk.at(s - 2).p - last_light.p);
      Spectrum bsdf_f = last_light.bsdf->eval_f(bsdf_wi, wo, BSDF_ALL);
      if(bsdf_f.is_black()) { return Spectrum(0.f); }

      return last_light.alpha * bsdf_f * camera_importance
        * (abs_dot(last_light.nn, wo) / (wo_dir_pdf * film_pdf));
    } else if(s >= 2 && t >= 2) {
      // Connect the light and camera subpaths.
      const Vertex& last_light = light_walk.at(s - 1);
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

  float BdptRenderer::path_weight(const Scene& scene, Vec2 film_res,
      const Light& light, const Camera& camera, Vec2 film_pos,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t) const
  {
    assert(t >= 1 && s + t >= 2);
    float inv_weight_sum = 1.f;

    float ri_light = 1.f;
    for(uint32_t i = 1; i <= s; ++i) {
      // alternative strategy with s - i light and t + 1 camera vertices
      // y is the vertex that is now sampled from camera instead of being
      // sampled from light
      const Vertex& y = light_walk.at(s - i);

      float y_bwd_pdf;
      if(i == 1 && t >= 2) {
        // pdf of sampling y from a surface vertex z
        const Vertex& z = camera_walk.at(t - 1);
        Vector wi = normalize(y.p - z.p);
        Vector camera_wo = normalize(camera_walk.at(t - 2).p - z.p);
        float wi_pdf = z.bsdf->light_f_pdf(wi, camera_wo, BSDF_ALL);
        y_bwd_pdf = wi_pdf * abs_dot(y.nn, wi) / length_squared(y.p - z.p);
      } else if(i == 1 && t == 1) {
        // pdf of sampling y directly from a point z on camera
        const Vertex& z = camera_walk.at(0);
        Vector wi = normalize(y.p - z.p);
        float wi_pdf = camera.ray_dir_importance_pdf(film_res, wi, z.p);
        y_bwd_pdf = wi_pdf * abs_dot(y.nn, wi) / length_squared(y.p - z.p);
      } else if(i >= 2) {
        // pdf of sampling y from a previous vertex of the light path is
        // already cached in the vertex y
        y_bwd_pdf = y.bwd_area_pdf;
      } else {
        assert(false && "Impossible combination");
      }
      assert(y_bwd_pdf >= 0.f && is_finite(y_bwd_pdf));

      ri_light = ri_light * y_bwd_pdf / y.fwd_area_pdf;
      if(ri_light == 0.f) { break; }
      if(!y.is_delta) {
        inv_weight_sum += ri_light;
      }
    }

    float ri_camera = 1.f;
    for(uint32_t i = 1; i < t; ++i) {
      // alternative strategy with s + i light and t - i camera vertices
      // z is the vertex that is now sampled from light instead of being sampled
      // from camera
      const Vertex& z = camera_walk.at(t - i);

      float z_bwd_pdf;
      if(i == 1 && s >= 2) {
        // pdf of sampling z from a surface vertex y
        const Vertex& y = light_walk.at(s - 1);
        Vector wo = normalize(z.p - y.p);
        Vector light_wi = normalize(light_walk.at(s - 2).p - y.p);
        float wo_pdf = y.bsdf->camera_f_pdf(wo, light_wi, BSDF_ALL);
        z_bwd_pdf = wo_pdf * abs_dot(z.nn, wo) / length_squared(z.p - y.p);
      } else if(i == 1 && s == 1) {
        // pdf of sampling z directly from a point y on light
        const Vertex& y = light_walk.at(0);
        Vector wo = normalize(z.p - y.p);
        float wo_pdf = light.ray_dir_radiance_pdf(scene, wo, y.p, y.nn);
        z_bwd_pdf = wo_pdf * abs_dot(z.nn, wo) / length_squared(z.p - y.p);
      } else if(i == 1 && s == 0) {
        // pdf of sampling z as a point on light
        if(&light == z.area_light) {
          // this is only plausible if the last camera vertex lies on the given
          // light
          z_bwd_pdf = light.ray_orig_radiance_pdf(scene, z.p);
        } else {
          z_bwd_pdf = 0.f;
        }
      } else if(i >= 2) {
        // pdf of sampling z from a previous camera vertex is cached in the
        // vertex
        z_bwd_pdf = z.bwd_area_pdf;
      } else {
        assert(false && "Impossible combination");
      }
      assert(z_bwd_pdf >= 0.f && is_finite(z_bwd_pdf));

      ri_camera = ri_camera * z_bwd_pdf / z.fwd_area_pdf;
      if(ri_camera == 0.f) { break; }
      if(i == t - 1 && !this->use_t1_paths) { continue; }
      if(!z.is_delta) {
        inv_weight_sum += ri_camera;
      }
    }

    (void)film_pos;
    assert(is_finite(inv_weight_sum));
    return 1.f / inv_weight_sum;
  }


  void BdptRenderer::init_debug_films() {
    for(uint32_t s = 0; s < 5; ++s) {
      for(uint32_t t = 1; t < 5; ++t) {
        for(bool weighted: {true, false}) {
          if(s + t < this->min_depth + 2) { continue; }
          if(s + t > this->max_depth + 2) { continue; }
          this->debug_films.emplace(std::piecewise_construct,
              std::forward_as_tuple(weighted | (s << 1) | (t << 9)),
              std::forward_as_tuple(
                this->film->x_res, this->film->y_res, this->film->filter));
        }
      }
    }
  }

  void BdptRenderer::save_debug_films() {
    for(auto& pair: this->debug_films) {
      auto key = pair.first;
      uint32_t s = (key >> 1) & 0xff;
      uint32_t t = (key >> 9) & 0xff;
      bool weighted = (key & 1) != 0;
      std::string path = this->debug_image_dir + "/bdpt_" +
        std::to_string(s) + "_" + std::to_string(t) +
        (weighted ? "_weighted" : "_contrib") + ".hdr";

      auto& film = pair.second;
      film.splat_scale = 1.f / float(this->iteration_count);
      auto image = film.to_image<PixelRgbFloat>();

      FILE* output = std::fopen(path.c_str(), "w");
      assert(output != nullptr);
      if(output != nullptr) {
        write_image_rgbe(output, image);
        std::fclose(output);
      }
    }
  }

  void BdptRenderer::store_debug_contrib(
      uint32_t s, uint32_t t, bool weighted,
      Vec2 film_pos, const Spectrum& contrib) const
  {
    auto iter = this->debug_films.find(weighted | (s << 1) | (t << 9));
    if(iter == this->debug_films.end()) { return; }

    Film& film = iter->second;
    film.add_splat(film_pos, contrib);
  }
}

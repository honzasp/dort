#include "dort/bdpt_renderer.hpp"
#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/lighting.hpp"
#include "dort/primitive.hpp"

namespace dort {
  void BdptRenderer::render(CtxG& ctx, Progress& progress) {
    this->preprocess(*this->scene);
    this->render_tiled(ctx, progress, this->iteration_count,
        [this](CtxG& ctx, Recti tile_rect, Recti tile_film_res,
            Film& file_film, Sampler& sampler, Progress& progress) 
        {
          this->render_tile(ctx, tile_rect, tile_film_res, file_film, sampler, progress);
        },
        [this](Film& film, uint32_t iter) {
          this->iteration(film, iter);
        });
    this->postprocess();
  }

  void BdptRenderer::preprocess(const Scene& scene) {
    this->light_distrib = compute_light_distrib(scene);
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      this->light_distrib_pdfs.insert(std::make_pair(
          scene.lights.at(i).get(), this->light_distrib.pdf(i)));
    }

    if(!this->debug_image_dir.empty()) {
      this->init_debug_films();
    }
  }

  void BdptRenderer::postprocess() {
    if(!this->debug_image_dir.empty()) {
      this->save_debug_films();
    }
  }

  void BdptRenderer::render_tile(CtxG&, Recti tile_rect, Recti tile_film_rect,
      Film& tile_film, Sampler& sampler, Progress& progress) const
  {
    StatTimer t(TIMER_RENDER_TILE);
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));

    for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
      for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
        sampler.start_pixel();
        for(uint32_t s = 0; s < sampler.samples_per_pixel; ++s) {
          sampler.start_pixel_sample();

          Vec2 pixel_pos = sampler.random_2d();
          Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
          Spectrum contrib = this->sample_path(*this->scene, film_pos, sampler);
          assert(is_finite(contrib));
          assert(is_nonnegative(contrib));
          if(is_finite(contrib) && is_nonnegative(contrib)) {
            Vec2 tile_film_pos = film_pos -
              Vec2(float(tile_film_rect.p_min.x), float(tile_film_rect.p_min.y));
            tile_film.add_sample(tile_film_pos, contrib);
          }
        }
      }
      if(progress.is_cancelled()) { return; }
    }
  }

  void BdptRenderer::iteration(Film& film, uint32_t iteration) {
    film.splat_scale = 1.f / (float(this->sampler->samples_per_pixel) *
        float(iteration + 1));
  }

  Spectrum BdptRenderer::sample_path(const Scene& scene,
      Vec2 film_pos, Sampler& sampler) const 
  {
    // TODO: use sampling patterns
    // TODO: correctly handle delta distributions in BSDFs
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    const Camera& camera = *this->camera;
    const Light* light;
    std::vector<Vertex> light_walk = this->random_light_walk(
        scene, light, sampler.rng);
    std::vector<Vertex> camera_walk = this->random_camera_walk(
        scene, film_pos, sampler.rng);

    Spectrum contrib(0.f);
    for(uint32_t s = 0; s <= light_walk.size(); ++s) {
      for(uint32_t t = 1; t <= camera_walk.size(); ++t) {
        if(t == 1 && !this->use_t1_paths) { continue; }
        if(s + t < this->min_depth + 2) { continue; }
        if(s + t > this->max_depth + 2) { continue; }

        const Light* new_light = light;
        Vec2 new_film_pos;
        Vertex new_first_light;
        Vertex new_first_camera;
        Spectrum path_contrib = this->path_contrib(scene, film_res, *light, camera,
            sampler.rng, light_walk, camera_walk, s, t,
            new_light, new_first_light, new_first_camera, new_film_pos);
        if(path_contrib.is_black()) { continue; }

        const Vertex& path_first_light =
          s == 1 ? new_first_light : light_walk.at(0);
        const Vertex& path_first_camera =
          t == 1 ? new_first_camera : camera_walk.at(0);
        Vec2 path_film_pos =
          t == 1 ? new_film_pos : film_pos;

        float path_weight = this->path_weight(scene, film_res,
            *new_light, camera, light_walk, camera_walk, s, t,
            path_first_light, path_first_camera);
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
    y0.fwd_pdf = light_pos_pdf;
    y0.bwd_pdf = SIGNALING_NAN;
    y0.alpha = light_radiance / (light_pos_pdf * light_pick_pdf);
    y0.is_delta = light.flags & LIGHT_DELTA;
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
      y.fwd_pdf = (bounces == 0 && (light.flags & LIGHT_DISTANT)) ? light_dir_pdf
        : fwd_dir_pdf * abs_dot(y.nn, wi) / dist_squared;
      y.bwd_pdf = SIGNALING_NAN;
      y.alpha = prev_y.alpha * alpha_scale;
      y.is_delta = bsdf_flags & BSDF_DELTA;
      if(y.alpha.is_black()) {
        break;
      }

      prev_y.bwd_pdf = bwd_dir_pdf * abs_dot(prev_y.nn, wi) / dist_squared;
      prev_bsdf_f = bsdf_f;
      fwd_dir_pdf = wo_pdf;
      light_ray = Ray(y.p, wo, isect.ray_epsilon);
      walk.push_back(std::move(y));
    }
    return walk;
  }

  std::vector<BdptRenderer::Vertex> BdptRenderer::random_camera_walk(
      const Scene& scene, Vec2 film_pos, Rng& rng) const
  {
    Ray ray;
    float ray_pos_pdf;
    float ray_dir_pdf;
    Spectrum importance = this->camera->sample_ray_importance(
        Vec2(float(this->film->x_res), float(this->film->y_res)), film_pos,
        ray, ray_pos_pdf, ray_dir_pdf, CameraSample(rng));

    std::vector<Vertex> walk;

    Vertex z0;
    z0.p = ray.orig;
    z0.p_epsilon = 0.f;
    z0.nn = Normal(ray.dir);
    z0.bsdf = nullptr;
    z0.area_light = nullptr;
    z0.fwd_pdf = ray_pos_pdf;
    z0.bwd_pdf = SIGNALING_NAN;
    z0.alpha = importance / ray_pos_pdf;
    z0.is_delta = false;
    walk.push_back(std::move(z0));

    Spectrum prev_bsdf_f(1.f);
    float fwd_dir_pdf = ray_dir_pdf;
    for(uint32_t bounces = 0; bounces <= this->max_depth; ++bounces) {
      Vertex& prev_z = walk.at(walk.size() - 1);
      if(prev_z.alpha.is_black()) { break; }

      Intersection isect;
      if(!scene.intersect(ray, isect)) { break; }

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      Vector wo = -ray.dir;
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
      z.fwd_pdf = fwd_dir_pdf * abs_dot(z.nn, wo) / dist_squared;;
      z.bwd_pdf = SIGNALING_NAN;
      z.alpha = prev_z.alpha * alpha_scale;
      z.is_delta = bsdf_flags & BSDF_DELTA;
      if(z.alpha.is_black() && !z.area_light) {
        break;
      }

      prev_z.bwd_pdf = bwd_dir_pdf * abs_dot(prev_z.nn, wo) / dist_squared;
      prev_bsdf_f = bsdf_f;
      fwd_dir_pdf = wi_pdf;
      ray = Ray(z.p, wi, isect.ray_epsilon);
      walk.push_back(std::move(z));
    }
    return walk;
  }

  Spectrum BdptRenderer::path_contrib(const Scene& scene, Vec2 film_res,
      const Light& light, const Camera& camera, Rng& rng,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t,
      const Light*& out_light,
      Vertex& out_first_light,
      Vertex& out_first_camera,
      Vec2& out_film_pos) const
  {
    const Vertex& last_camera = camera_walk.at(t - 1);

    if(s == 1 && t == 1) {
      // Connect the camera vertex to a new light vertex.
      if(!(this->camera->flags & CAMERA_DIR_BY_POS_DELTA)) {
        // pick a point z on camera, sample light direction from this point,
        // sample film position from this direction
        float camera_p_pdf;
        Point camera_p = camera.sample_point(camera_p_pdf, CameraSample(rng));
        if(camera_p_pdf == 0.f) { return Spectrum(0.f); }

        Vector wi;
        Normal light_nn;
        float wi_dir_pdf;
        ShadowTest shadow;
        Spectrum radiance = light.sample_pivot_radiance(camera_p, 0.f,
            wi, wi_dir_pdf, shadow, LightSample(rng));
        if(wi_dir_pdf == 0.f || radiance.is_black()) { return Spectrum(0.f); }

        Spectrum importance = camera.eval_importance(film_res,
            camera_p, wi, out_film_pos);
        if(importance.is_black()) { return Spectrum(0.f); }

        float light_pdf = this->light_distrib_pdfs.at(&light);
        if(light_pdf == 0.f) { return Spectrum(0.f); }
        if(!shadow.visible(scene)) { return Spectrum(0.f); }

        // TODO: initialize out_first_light and out_first_camera

        return (importance * radiance) / (camera_p_pdf * wi_dir_pdf * light_pdf);
      } else {
        // TODO: pick a point y on light, sample camera point, direction and
        // film position to look at this point, evaluate radiance from the light
        // in the direction
        out_film_pos = Vec2();
        return Spectrum(0.f);
      }
    } else if(s == 0 && t >= 2) {
      // TODO: use the background light if any
      // Use only the camera path, provided that the last vertex is emissive.
      if(last_camera.area_light == nullptr) { return Spectrum(0.f); }
      Spectrum emitted_radiance = last_camera.area_light->eval_radiance(
          last_camera.p, last_camera.nn, camera_walk.at(t - 2).p);
      if(emitted_radiance.is_black()) { return Spectrum(0.f); }

      out_light = last_camera.area_light;
      return emitted_radiance * last_camera.alpha;
    } else if(s == 1) {
      // Connect the camera subpath to a new light vertex.
      uint32_t light_i = this->light_distrib.sample(rng.uniform_float());
      float light_pick_pdf = this->light_distrib.pdf(light_i);
      const Light& light = *scene.lights.at(light_i);
      out_light = &light;
      if(light_pick_pdf == 0.f) { return Spectrum(0.f); }

      Vector wi;
      float wi_dir_pdf;
      ShadowTest shadow;
      Point light_p;
      Normal light_nn;
      float light_p_epsilon;
      Spectrum light_radiance = light.sample_pivot_radiance(
          last_camera.p, last_camera.p_epsilon,
          wi, light_p, light_nn, light_p_epsilon,
          wi_dir_pdf, shadow, LightSample(rng));
      if(light_radiance.is_black() || wi_dir_pdf == 0.f) {
        return Spectrum(0.f);
      }
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector bsdf_wo = normalize(camera_walk.at(t - 2).p - last_camera.p);
      Spectrum bsdf_f = last_camera.bsdf->eval_f(wi, bsdf_wo, BSDF_ALL);
      if(bsdf_f.is_black()) { return Spectrum(0.f); }

      if(light.flags & LIGHT_DISTANT) {
        // a "clever hack" to store the incident direction
        out_first_light.p = last_camera.p + wi;
        out_first_light.p_epsilon = SIGNALING_NAN;
        out_first_light.nn = Normal(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
        out_first_light.fwd_pdf = wi_dir_pdf;
      } else {
        out_first_light.p = light_p;
        out_first_light.p_epsilon = light_p_epsilon;
        out_first_light.nn = light_nn;
        out_first_light.fwd_pdf = wi_dir_pdf * abs_dot(light_nn, wi)
          / length_squared(light_p - last_camera.p);
      }
      out_first_light.bsdf = nullptr;
      out_first_light.area_light = nullptr;
      out_first_light.alpha = Spectrum(SIGNALING_NAN);
      out_first_light.is_delta = light.flags & LIGHT_DELTA;
      out_first_light.bwd_pdf = last_camera.bsdf->light_f_pdf(wi, bsdf_wo, BSDF_ALL);

      return last_camera.alpha * bsdf_f * light_radiance
        * (abs_dot(last_camera.nn, wi) / (light_pick_pdf * wi_dir_pdf));
    } else if(t == 1) {
      // Connect the light subpath to a new camera vertex.
      const Vertex& last_light = light_walk.at(s - 1);
      Point camera_p;
      float camera_p_pdf;
      ShadowTest shadow;
      Spectrum camera_importance = camera.sample_pivot_importance(film_res,
          last_light.p, last_light.p_epsilon,
          camera_p, out_film_pos, camera_p_pdf,
          shadow, CameraSample(rng));
      if(camera_importance.is_black() || camera_p_pdf == 0.f) {
        return Spectrum(0.f);
      }
      if(!shadow.visible(scene)) { return Spectrum(0.f); }

      Vector wi = normalize(light_walk.at(s - 2).p - last_light.p);
      Vector wo = normalize(camera_p - last_light.p);
      Spectrum bsdf_f = last_light.bsdf->eval_f(wi, wo, BSDF_ALL);
      if(bsdf_f.is_black()) { return Spectrum(0.f); }

      out_first_camera.p = camera_p;
      out_first_camera.p_epsilon = 0.f;
      out_first_camera.nn = Normal(SIGNALING_NAN, SIGNALING_NAN, SIGNALING_NAN);
      out_first_camera.bsdf = nullptr;
      out_first_camera.area_light = nullptr;
      out_first_camera.fwd_pdf = camera_p_pdf;
      out_first_camera.bwd_pdf = SIGNALING_NAN;
      out_first_camera.alpha = camera_importance;
      out_first_camera.is_delta = false;

      return last_light.alpha * bsdf_f * camera_importance
        * (abs_dot(last_light.nn, wo) 
        / (length_squared(camera_p - last_light.p) * camera_p_pdf));
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
      const Light& light, const Camera& camera,
      const std::vector<Vertex>& light_walk,
      const std::vector<Vertex>& camera_walk,
      uint32_t s, uint32_t t,
      const Vertex& first_light,
      const Vertex& first_camera) const
  {
    assert(t >= 1 && s + t >= 2);
    auto light_at = [&](uint32_t i) -> const Vertex& {
      assert(i < s); return i == 0 ? first_light : light_walk.at(i);
    };
    auto camera_at = [&](uint32_t i) -> const Vertex& {
      assert(i < t); return i == 0 ? first_camera : camera_walk.at(i);
    };
    auto vertex_at = [&](uint32_t i) -> const Vertex& {
      assert(i < s + t);
      if(i < s) {
        return light_at(i);
      } else {
        return camera_at(t - (i - s + 1));
      }
    };

    if(s + t == 2) {
      // Special cases for the direct paths
      // TODO: this is very crude
      if(light.flags & LIGHT_AREA) {
        // let area lights be intersected by the camera rays
        return t == 2 ? 1.f : 0.f;
      } else {
        // non-area lights will sample their camera vertices
        return t == 1 ? 1.f : 0.f;
      }
    }

    // precompute the solid angle or area pdf of sampling x0 from x1 using
    // sample_pivot_radiance()
    float x0_pivot_pdf;
    if(s == 1) {
      // if we sampled a new x0, the pdf is stored in the vertex
      x0_pivot_pdf = vertex_at(0).fwd_pdf;
    } else {
      // otherwise the pdf must be computed
      const Vertex& x0 = vertex_at(0);
      const Vertex& x1 = vertex_at(1);
      Vector wi = normalize(x0.p - x1.p);
      float x0_dir_pdf = light.pivot_radiance_pdf(wi, x1.p);
      if(light.flags & LIGHT_DISTANT) {
        x0_pivot_pdf = x0_dir_pdf;
      } else {
        x0_pivot_pdf = x0_dir_pdf * abs_dot(x0.nn, wi) / length_squared(x0.p - x1.p);
      }
    }

    // computes the pdf of sampling point x[i] from x[i-1] (i.e., from light)
    // if i == 0 and the light is distant, this is solid angle pdf, otherwise it
    // is area pdf.
    auto pdf_light = [&](uint32_t i) {
      if(i == 0) {
        // pdf of sampling x0 from x1 using sample_pivot_radiance()
        return x0_pivot_pdf;
      } else if(i == 1) {
        // area pdf of sampling x1 from x0 (cancelling the pdf of sampling x0
        // from x1) using sample_pivot_radiance()
        if(x0_pivot_pdf == 0.f) { return 0.f; }

        const Vertex& x0 = vertex_at(i-1);
        const Vertex& x1 = vertex_at(i);
        Vector wi = normalize(x0.p - x1.p);
        float ray_pdf;
        if(s >= 2) {
          // the pdfs are stored in the vertices
          if(light.flags & LIGHT_DISTANT) {
            // the pdfs of distant lights are "reversed", so x0.fwd_pdf is the
            // area pdf of x1 w.r.t. the plane perpendicular to wi, while
            // x1.fwd_pdf is the directional density corresponding to wi
            float x0_dir_pdf = x1.fwd_pdf;
            float x1_area_pdf = x0.fwd_pdf * abs_dot(wi, x1.nn);
            ray_pdf = x0_dir_pdf * x1_area_pdf;
          } else {
            // for non-distant lights, both pdfs w.r.t. area are already stored
            // in the vertices
            ray_pdf = x0.fwd_pdf * x1.fwd_pdf;
          }
        } else {
          float ray_angle_pdf = light.ray_radiance_pdf(scene, x0.p, -wi, x0.nn);
          if(light.flags & LIGHT_DISTANT) {
            ray_pdf = ray_angle_pdf;
          } else {
            // the ray pdf must be converted from area*solidangle to area*area
            ray_pdf = ray_angle_pdf * abs_dot(x1.nn, wi) / length_squared(x0.p - x1.p);
          }
        }

        return ray_pdf / x0_pivot_pdf;
      } else if(i >= 2 && i < s) {
        // the BSDF sampling probability is cached in the light vertices
        return vertex_at(i).fwd_pdf;
      } else if(i >= 2 && i == s && t != 1) {
        // the pdf of sampling the last surface camera vertex must be computed
        const Vertex& xo = vertex_at(i);
        const Vertex& x = vertex_at(i-1);
        const Vertex& xi = vertex_at(i-2);
        Vector bsdf_wi = normalize(xi.p - x.p);
        Vector bsdf_wo = normalize(xo.p - x.p);
        float wo_dir_pdf = x.bsdf->camera_f_pdf(bsdf_wo, bsdf_wi, BSDF_ALL);
        return wo_dir_pdf * abs_dot(bsdf_wo, xo.nn) / length_squared(xo.p - x.p);
      } else if(i >= 2 && i > s && i < s + t - 1) {
        // the backward BSDF sampling pdf is cached in the camera vertices
        return vertex_at(i).bwd_pdf;
      } else {
        assert(false && "Invalid combination in pdf_light()");
        return 0.f;
      }
    };

    // computes the pdf of sampling point x[i] from x[i+1] (i.e., from camera)
    // if i == 0 and the light is distant, this is solid angle pdf, otherwise it
    // is area pdf.
    auto pdf_camera = [&](uint32_t i) {
      if(i == s + t - 2) {
        const Vertex& z1 = vertex_at(s + t - 2);
        const Vertex& z0 = vertex_at(s + t - 1);
        Vector wi = normalize(z1.p - z0.p);

        float ray_area_pdf;
        if(t >= 2) {
          // the path was sampled using sample_ray_importance() and the pdfs are
          // stored in the vertices
          ray_area_pdf = z0.fwd_pdf * z1.fwd_pdf;
        } else {
          // the camera vertex was freshly sampled
          float ray_pdf = camera.ray_importance_pdf(film_res, z0.p, wi);
          ray_area_pdf = ray_pdf * abs_dot(wi, z1.nn) / length_squared(z1.p - z0.p);
        }

        float pivot_area_pdf;
        if(t == 1) {
          // the path was sampled using sample_pivot_importance() and the pdf is
          // stored in z0
          pivot_area_pdf = z0.fwd_pdf;
        } else {
          pivot_area_pdf = camera.pivot_importance_pdf(film_res, z0.p, z1.p);
        }

        return ray_area_pdf / pivot_area_pdf;
      } else if(i < s + t - 2 && i >= s) {
        // the forward BSDF sampling pdf is cached in the camera vertices
        return vertex_at(i).fwd_pdf;
      } else if(i == s - 1 && t >= 2) {
        // the pdf of sampling the last light vertex must be computed
        if(i == 0 && (light.flags & LIGHT_DELTA)) {
          // delta light cannot be intersected randomly
          return 0.f;
        }
        const Vertex& xi = vertex_at(i);
        const Vertex& x = vertex_at(i+1);
        const Vertex& xo = vertex_at(i+2);
        Vector bsdf_wi = normalize(xi.p - x.p);
        Vector bsdf_wo = normalize(xo.p - x.p);
        float wi_dir_pdf = x.bsdf->light_f_pdf(bsdf_wi, bsdf_wo, BSDF_ALL);
        if(i == 0 && (light.flags & LIGHT_DISTANT)) {
          return wi_dir_pdf;
        } else {
          return wi_dir_pdf * abs_dot(xi.nn, bsdf_wi) / length_squared(xi.p - x.p);
        }
      } else if(i < s - 1) {
        assert(!(s == 1 && i == 0));
        // the backward BSDF sampling pdf is cached in the light vertices
        return vertex_at(i).bwd_pdf;
      } else {
        assert(false && "Invalid combination in pdf_camera()");
      }
    };


    float inv_weight_sum = 1.f;

    float r_light = 1.f;
    for(uint32_t j = 1; j <= t - 1; ++j) {
      // alternative strategy using s + j light vertices and t - j camera
      // vertices
      float fwd_pdf = pdf_light(s + j - 1);
      float bwd_pdf = pdf_camera(s + j - 1);
      assert(fwd_pdf >= 0.f && is_finite(fwd_pdf));
      assert(bwd_pdf >= 0.f && is_finite(bwd_pdf));
      if(bwd_pdf == 0.f) { break; }
      r_light *= fwd_pdf / bwd_pdf;

      if(r_light == 0.f) { break; }
      if(vertex_at(s + j - 1).is_delta) { continue; }
      if(vertex_at(s + j).is_delta) { continue; }
      inv_weight_sum += r_light;
    }

    float r_camera = 1.f;
    for(uint32_t j = 1; j <= s; ++j) {
      // alternative strategy using s - j light vertices and t + j camera
      // vertices
      float fwd_pdf = pdf_camera(s - j);
      float bwd_pdf = pdf_light(s - j);
      assert(fwd_pdf >= 0.f && is_finite(fwd_pdf));
      assert(bwd_pdf >= 0.f && is_finite(bwd_pdf));
      if(bwd_pdf == 0.f) { break; }
      r_camera *= fwd_pdf / bwd_pdf;

      if(r_camera == 0.f) { break; }
      if(vertex_at(s - j).is_delta) { continue; }
      if(s - j > 0 && vertex_at(s - j - 1).is_delta) { continue; }
      inv_weight_sum += r_camera;
    }

    assert(inv_weight_sum >= 1.f && is_finite(inv_weight_sum));
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

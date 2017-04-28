#include <shared_mutex>
#include "dort/camera.hpp"
#include "dort/film.hpp"
#include "dort/primitive.hpp"
#include "dort/vcm_renderer.hpp"

namespace dort {
  void VcmRenderer::render(CtxG& ctx, Progress& progress) {
    if(this->scene->lights.empty()) { return; }

    this->light_distrib = compute_light_distrib(*this->scene);
    this->background_light_distrib = compute_light_distrib(*this->scene, true);
    for(uint32_t i = 0; i < scene->lights.size(); ++i) {
      this->light_distrib_pdfs.insert(std::make_pair(
          scene->lights.at(i).get(), this->light_distrib.pdf(i)));
    }
    this->init_debug_films();

    std::vector<Photon> photons;
    for(uint32_t i = 0; i < this->iteration_count; ++i) {
      photons = this->iteration(ctx, i, std::move(photons));
      progress.set_percent_done(float(i + 1) / float(this->iteration_count));
      if(progress.is_cancelled()) { break; }
      this->film->splat_scale = 1.f / (float(i + 1));
    }

    this->save_debug_films();
  }

  std::vector<VcmRenderer::Photon> VcmRenderer::iteration(CtxG& ctx,
      uint32_t idx, std::vector<Photon> prev_photons) 
  {
    float radius = this->initial_radius
      * pow(float(idx + 1), 0.5f * (this->alpha - 1.f));
    uint32_t path_count = this->film->res.x * this->film->res.y;
    float eta_vcm = PI * square(radius) * float(path_count);

    IterationState iter_state;
    iter_state.idx = idx;
    iter_state.path_count = path_count;
    iter_state.radius = radius;
    iter_state.mis_vm_weight = this->use_vm ? eta_vcm : 0.f;
    iter_state.mis_vc_weight = this->use_vc ? 1.f / eta_vcm : 0.f;
    iter_state.vm_normalization = 1.f / eta_vcm
      * float(this->iteration_count) / float(this->iteration_count - 1);
    if(this->use_vm) {
      iter_state.photon_tree = KdTree<PhotonKdTraits>(std::move(prev_photons));
    }

    std::vector<Photon> photons;
    std::atomic<uint32_t> photon_count(0);
    std::shared_timed_mutex photons_mutex;

    auto ensure_photons_size = [&](uint32_t min_size) {
      std::unique_lock<decltype(photons_mutex)> resize_lock(photons_mutex);
      if(min_size <= photons.size()) { return; }
      photons.resize(min_size);
      photons.resize(photons.capacity());
    };

    auto add_photons = [&](const std::vector<Photon>& block) {
      std::shared_lock<decltype(photons_mutex)> write_lock(photons_mutex);
      uint32_t block_size = block.size();
      uint32_t block_begin = photon_count.fetch_add(block_size);
      if(block_begin + block_size > photons.size()) {
        write_lock.unlock();
        ensure_photons_size(block_begin + block_size);
        write_lock.lock();
      }

      for(uint32_t i = 0; i < block_size; ++i) {
        photons.at(block_begin + i) = block.at(i);
      }
    };

    this->iteration_tiled_per_job(ctx,
      [&](Film& tile, Recti tile_rect, Recti tile_film_rect, Sampler& sampler)
    {
      std::vector<Photon> photons_block;
      std::vector<PathVertex> light_vertices;
      for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
        for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
          LightPathState light_path = this->light_walk(iter_state,
              light_vertices, photons_block, sampler);
          Vec2 film_pos = Vec2(x, y) + sampler.random_2d();
          Spectrum contrib = this->camera_walk(iter_state, light_path,
              light_vertices, film_pos, sampler);
          tile.add_sample(film_pos - Vec2(tile_film_rect.p_min), contrib);
          light_vertices.clear();
        }
      }
      add_photons(photons_block);
    });

    photons.resize(photon_count.load());
    return photons;
  }

  VcmRenderer::LightPathState VcmRenderer::light_walk(const IterationState& iter_state,
      std::vector<PathVertex>& light_vertices,
      std::vector<Photon>& photons, Sampler& sampler)
  {
    // pick a light and sample the first ray
    uint32_t light_i = this->light_distrib.sample(sampler.random_1d());
    float light_pick_pdf = this->light_distrib.pdf(light_i);
    const Light& light = *scene->lights.at(light_i);

    Ray light_ray;
    Normal light_nn;
    float light_pos_pdf, light_dir_pdf;
    Spectrum light_radiance = light.sample_ray_radiance(*this->scene,
        light_ray, light_nn, light_pos_pdf, light_dir_pdf, LightRaySample(sampler.rng));
    float light_ray_pdf = light_pos_pdf * light_dir_pdf * light_pick_pdf;

    Spectrum throughput = light_radiance 
      * (abs_dot(light_nn, normalize(light_ray.dir)) / light_ray_pdf);
    float fwd_bsdf_dir_pdf = SIGNALING_NAN;
    bool fwd_bsdf_delta = false;

    for(uint32_t bounces = 0;; ++bounces) {
      if(light_ray_pdf == 0.f) { break; }
      if(throughput.is_black()) { break; }

      Intersection isect;
      if(!this->scene->intersect(light_ray, isect)) {
        break;
      }

      PathVertex y;
      y.p = isect.world_diff_geom.p;
      y.p_epsilon = isect.ray_epsilon;
      y.w = -normalize(light_ray.dir);
      y.nn = isect.world_diff_geom.nn;
      y.throughput = throughput;
      y.bsdf = isect.get_bsdf();

      // compute the MIS quantities
      if(bounces == 0) {
        float pivot_dir_pdf = light.pivot_radiance_pdf(y.w, y.p);
        // equations (31), (32), (33), (49), (50)
        if(light.flags & LIGHT_DISTANT) {
          y.d_vcm = pivot_dir_pdf 
            / (light_dir_pdf * light_pos_pdf * abs_dot(y.nn, y.w));
        } else {
          y.d_vcm = pivot_dir_pdf * abs_dot(light_nn, y.w) 
            / (light_dir_pdf * light_pos_pdf * abs_dot(y.nn, y.w));
        }

        if(light.flags & LIGHT_DELTA) {
          y.d_vc = 0.f;
        } else if(light.flags & LIGHT_DISTANT) {
          y.d_vc = 1.f / (light_ray_pdf * abs_dot(y.nn, y.w));
        } else {
          y.d_vc = abs_dot(light_nn, y.w) /
            (light_ray_pdf * abs_dot(y.nn, y.w));
        }
        y.d_vm = y.d_vc * iter_state.mis_vc_weight;
      } else if(fwd_bsdf_delta) {
        // equations (53), (54), (55)
        const auto& yp = light_vertices.at(bounces - 1);
        y.d_vcm = 0.f;
        y.d_vc = abs_dot(yp.nn, y.w) / abs_dot(y.nn, y.w) * yp.d_vc;
        y.d_vm = abs_dot(yp.nn, y.w) / abs_dot(y.nn, y.w) * yp.d_vm;
      } else {
        const auto& yp = light_vertices.at(bounces - 1);
        float bwd_bsdf_dir_pdf = yp.bsdf->light_f_pdf(yp.w, -y.w, BSDF_ALL);
        // equations (34), (35), (36)
        y.d_vcm = length_squared(yp.p - y.p) /
          (fwd_bsdf_dir_pdf * abs_dot(y.nn, y.w));
        y.d_vc = abs_dot(yp.nn, y.w) / (abs_dot(y.nn, y.w) * fwd_bsdf_dir_pdf)
          * (iter_state.mis_vm_weight + yp.d_vcm + bwd_bsdf_dir_pdf * yp.d_vc);
        y.d_vm = abs_dot(yp.nn, y.w) / (abs_dot(y.nn, y.w) * fwd_bsdf_dir_pdf)
          * (1.f + yp.d_vcm * iter_state.mis_vc_weight + bwd_bsdf_dir_pdf * yp.d_vm);
      }

      if(this->use_vc && bounces + 3 <= this->max_length &&
          bounces + 3 >= this->min_length) 
      {
        this->connect_to_camera(iter_state, y, bounces, *this->film, sampler);
      }

      // the path is too long to be ever accepted
      if(bounces + 2 >= this->max_length) { break; }

      // store the photon
      Photon photon;
      photon.p = y.p;
      photon.wi = y.w;
      photon.nn = y.nn;
      photon.throughput = throughput;
      photon.d_vcm = y.d_vcm;
      photon.d_vc = y.d_vc;
      photon.d_vm = y.d_vm;
      photon.bounces = bounces;
      photons.push_back(photon);

      // bounce
      Vector bsdf_wo;
      float bsdf_wo_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = y.bsdf->sample_camera_f(y.w, BSDF_ALL,
          bsdf_wo, bsdf_wo_pdf, bsdf_flags, BsdfSample(sampler.rng));
      if(bsdf_f.is_black() || bsdf_wo_pdf == 0.f) { break; }

      light_ray = Ray(y.p, bsdf_wo, y.p_epsilon);
      fwd_bsdf_dir_pdf = bsdf_wo_pdf;
      fwd_bsdf_delta = bsdf_flags & BSDF_DELTA;
      throughput *= bsdf_f * (abs_dot(y.nn, bsdf_wo) / bsdf_wo_pdf);
      light_vertices.push_back(std::move(y));
    }

    LightPathState path;
    path.light = &light;
    path.light_pick_pdf = light_pick_pdf;
    return path;
  }

  Spectrum VcmRenderer::camera_walk(const IterationState& iter_state,
      const LightPathState& light_path,
      const std::vector<PathVertex>& light_vertices,
      Vec2 film_pos, Sampler& sampler)
  {
    Vec2 film_res(this->film->res);
    Ray camera_ray;
    float camera_pos_pdf, camera_dir_pdf;
    Spectrum camera_importance = this->camera->sample_ray_importance(film_res, film_pos,
        camera_ray, camera_pos_pdf, camera_dir_pdf, CameraSample(sampler.rng));

    float camera_ray_pdf = camera_pos_pdf * camera_dir_pdf;
    if(camera_ray_pdf == 0.f || camera_importance.is_black()) { return Spectrum(0.f); }

    Spectrum throughput = camera_importance / camera_ray_pdf;
    float fwd_bsdf_dir_pdf = SIGNALING_NAN;
    bool fwd_bsdf_delta = false;
    PathVertex zp;
    zp.p = camera_ray.orig;
    zp.throughput = throughput;
    Spectrum film_contrib(0.f);

    for(uint32_t bounces = 0;; ++bounces) {
      if(throughput.is_black()) { break; }
      Intersection isect;
      if(!this->scene->intersect(camera_ray, isect)) {
        if(bounces + 2 >= this->min_length && bounces + 2 <= this->max_length) {
          // VC, s = 0, t >= 2, background light
          film_contrib += this->connect_to_background_light(
            camera_ray, zp, throughput, film_pos, bounces, sampler);
        }
        break;
      }

      PathVertex z;
      z.p = isect.world_diff_geom.p;
      z.p_epsilon = isect.ray_epsilon;
      z.w = -normalize(camera_ray.dir);
      z.nn = isect.world_diff_geom.nn;
      z.throughput = throughput;
      z.bsdf = isect.get_bsdf();

      // compute the MIS quantities
      if(bounces == 0) {
        // equations (31), (32), (33)
        float pivot_area_pdf = this->camera->pivot_importance_pdf(film_res,
            camera_ray.orig, z.p);
        z.d_vcm = pivot_area_pdf * length_squared(z.p - camera_ray.orig)
          / (camera_ray_pdf * abs_dot(z.nn, z.w));
        z.d_vc = 0.f;
        z.d_vm = 0.f;
      } else if(fwd_bsdf_delta) {
        // equations (53), (54), (55)
        z.d_vcm = 0.f;
        z.d_vc = abs_dot(zp.nn, z.w) / abs_dot(z.nn, z.w) * zp.d_vc;
        z.d_vm = abs_dot(zp.nn, z.w) / abs_dot(z.nn, z.w) * zp.d_vm;
      } else {
        float bwd_bsdf_dir_pdf = zp.bsdf->camera_f_pdf(zp.w, -z.w, BSDF_ALL);
        // equations (34), (35), (36)
        z.d_vcm = length_squared(zp.p - z.p) /
          (fwd_bsdf_dir_pdf * abs_dot(z.nn, z.w));
        z.d_vc = abs_dot(zp.nn, z.w) / (abs_dot(z.nn, z.w) * fwd_bsdf_dir_pdf)
          * (iter_state.mis_vm_weight + zp.d_vcm + bwd_bsdf_dir_pdf * zp.d_vc);
        z.d_vm = abs_dot(zp.nn, z.w) / (abs_dot(z.nn, z.w) * fwd_bsdf_dir_pdf)
          * (1.f + zp.d_vcm * iter_state.mis_vc_weight + bwd_bsdf_dir_pdf * zp.d_vm);
      }

      if(this->use_vm && bounces + 2 <= this->max_length) {
        // VM, s >= 2, t >= 2
        film_contrib += this->merge_with_photons(iter_state, z, film_pos, bounces);
      }

      if(this->use_vc && bounces + 3 <= this->max_length) {
        // VC, s >= 2, t >= 2
        film_contrib += this->connect_to_light_vertices(iter_state, z,
            light_vertices, film_pos, bounces);
      }

      if(this->use_vc && bounces + 3 <= this->max_length &&
          bounces + 3 >= this->min_length) 
      {
        // VC, s = 1, t >= 2
        film_contrib += this->connect_to_light(
            iter_state, light_path, z, film_pos, bounces, sampler);
      }

      if(this->use_vc && bounces + 2 <= this->max_length &&
          bounces + 2 >= this->min_length) 
      {
        const Light* area_light = isect.get_area_light();
        if(area_light != nullptr) {
          // VC, s = 0, t >= 2, area light
          film_contrib += this->connect_area_light(zp, z, area_light, film_pos, bounces);
        }
      }

      // the path is too long to be ever accepted
      if(bounces + 2 >= this->max_length) { break; }

      // bounce
      Vector bsdf_wi;
      float bsdf_wi_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = z.bsdf->sample_light_f(z.w, BSDF_ALL,
          bsdf_wi, bsdf_wi_pdf, bsdf_flags, BsdfSample(sampler.rng));
      if(bsdf_f.is_black() || bsdf_wi_pdf == 0.f) { break; }

      camera_ray = Ray(z.p, bsdf_wi, isect.ray_epsilon);
      fwd_bsdf_dir_pdf = bsdf_wi_pdf;
      fwd_bsdf_delta = bsdf_flags & BSDF_DELTA;
      throughput *= bsdf_f * (abs_dot(z.nn, bsdf_wi) / bsdf_wi_pdf);
      zp = std::move(z);
    }

    return film_contrib;
  }

  void VcmRenderer::connect_to_camera(const IterationState& iter_state,
      const PathVertex& y, uint32_t bounces, Film& film, Sampler& sampler) const 
  {
    // connect vertex y to camera (VC, s >= 2, t = 1)
    Vec2 film_res(float(this->film->res.x), float(this->film->res.y));

    Point camera_p;
    Vec2 film_pos;
    float camera_p_pdf;
    ShadowTest shadow;
    Spectrum importance = this->camera->sample_pivot_importance(film_res,
        y.p, y.p_epsilon, camera_p, film_pos, camera_p_pdf,
        shadow, CameraSample(sampler.rng));
    if(importance.is_black() || camera_p_pdf == 0.f) { return; }

    Vector camera_wi = normalize(y.p - camera_p);
    Spectrum bsdf_f = y.bsdf->eval_f(y.w, -camera_wi, BSDF_ALL);
    if(bsdf_f.is_black()) { return; }

    float camera_ray_pdf = this->camera->ray_importance_pdf(film_res,
      camera_p, camera_wi);
    float bwd_bsdf_dir_pdf = y.bsdf->light_f_pdf(y.w, -camera_wi, BSDF_ALL);
    float dist_squared = length_squared(y.p - camera_p);

    // equation (46), (37)
    float w_light = camera_ray_pdf * abs_dot(y.nn, camera_wi)
      / (camera_p_pdf * dist_squared)
      * (iter_state.mis_vm_weight + y.d_vcm + bwd_bsdf_dir_pdf * y.d_vc);
    float weight = 1.f / (1.f + w_light);

    Spectrum contrib = y.throughput * bsdf_f * importance 
      * (abs_dot(y.nn, camera_wi) / (dist_squared * camera_p_pdf));
    if(!contrib.is_black() && shadow.visible(*this->scene)) {
      film.add_splat(film_pos, contrib * weight);
      this->store_debug_weighted_contrib(bounces + 2, 1, false,
          film_pos, contrib, weight);
    }
  }

  Spectrum VcmRenderer::connect_to_background_light(const Ray& camera_ray,
      const PathVertex& zp, Spectrum throughput,
      Vec2 film_pos, uint32_t bounces, Sampler& sampler) const
  {
    // "intersect" a background light (VC, s = 0, t >= 2, distant light)
    uint32_t light_i = this->background_light_distrib.sample(sampler.random_1d());
    float bg_light_pick_pdf = this->background_light_distrib.pdf(light_i);
    if(bg_light_pick_pdf == 0.f) { return Spectrum(0.f); }

    const Light& light = *this->scene->background_lights.at(light_i);
    assert(light.flags & LIGHT_DISTANT);
    Spectrum radiance = light.background_radiance(camera_ray);
    Spectrum contrib = throughput * radiance / bg_light_pick_pdf;
    if(contrib.is_black()) { return Spectrum(0.f); }

    if(bounces == 0) {
      // special case for (VC, s = 0, t = 2), no weighting
      this->store_debug_weighted_contrib(0, bounces + 2, false,
          film_pos, contrib, 1.f);
      return contrib;
    } else {
      Vector light_wi = normalize(camera_ray.dir);
      float pivot_dir_pdf = light.pivot_radiance_pdf(light_wi, zp.p);
      float ray_pdf = light.ray_radiance_pdf(*this->scene, zp.p, -light_wi, Normal());
      float light_pick_pdf = this->light_distrib_pdfs.at(&light);
      float w_camera = pivot_dir_pdf * light_pick_pdf * zp.d_vcm
        + ray_pdf * light_pick_pdf * zp.d_vc;
      float weight = 1.f / (w_camera + 1.f);

      this->store_debug_weighted_contrib(0, bounces + 2, false,
          film_pos, contrib, weight);
      return contrib * weight;
    }
  }

  Spectrum VcmRenderer::merge_with_photons(const IterationState& iter_state,
      const PathVertex& z, Vec2 film_pos, uint32_t bounces) const
  {
    // merge vertex z with photons (VM, s >= 2, t >= 2)
    // TODO: the radius from the previous iteration, baked in the photon MIS
    // quantities, is not compatible with the radius from the current
    // iteration!
    Spectrum photon_sum(0.f);
    iter_state.photon_tree.lookup(z.p, square(iter_state.radius),
      [&](const Photon& photon, float, float radius_square) {
        if(bounces + photon.bounces + 3 < this->min_length ||
            bounces + photon.bounces + 3 > this->max_length ||
            dot(photon.nn, z.nn) < 0.7f)
        {
          return radius_square;
        }

        Spectrum bsdf_f = z.bsdf->eval_f(photon.wi, z.w, BSDF_ALL);
        if(bsdf_f.is_black()) { return radius_square; }

        float bwd_light_dir_pdf = z.bsdf->light_f_pdf(photon.wi, -z.w, BSDF_ALL);
        float bwd_camera_dir_pdf = z.bsdf->camera_f_pdf(z.w, photon.wi, BSDF_ALL);

        // equations (38), (39)
        float w_light = photon.d_vcm * iter_state.mis_vc_weight
          + bwd_light_dir_pdf * photon.d_vm;
        float w_camera = z.d_vcm * iter_state.mis_vc_weight
          + bwd_camera_dir_pdf * z.d_vm;
        float weight = 1.f / (w_light + w_camera + 1.f);

        if(!this->debug_image_dir.empty()) {
          Spectrum contrib = photon.throughput * bsdf_f * z.throughput 
            * iter_state.vm_normalization;
          this->store_debug_weighted_contrib(photon.bounces + 2, bounces + 2, true,
              film_pos, contrib, weight);
        }

        photon_sum += bsdf_f * photon.throughput * weight;
        return radius_square;
      });

    return photon_sum * z.throughput * iter_state.vm_normalization;
  }

  Spectrum VcmRenderer::connect_to_light_vertices(const IterationState& iter_state,
      const PathVertex& z, const std::vector<PathVertex>& light_vertices,
      Vec2 film_pos, uint32_t bounces) const
  {
    Spectrum contrib_sum(0.f);
    // connect vertex z to light vertices (VC, s >= 2, t >= 2)
    for(uint32_t light_i = 0; light_i < light_vertices.size(); ++light_i) {
      if(bounces + light_i + 4 < this->min_length) { continue; }
      if(bounces + light_i + 4 > this->max_length) { break; }
      const PathVertex& y = light_vertices.at(light_i);

      Vector connect_wi = normalize(y.p - z.p);
      Spectrum light_bsdf_f = y.bsdf->eval_f(y.w, -connect_wi, BSDF_ALL);
      if(light_bsdf_f.is_black()) { continue; }
      Spectrum camera_bsdf_f = z.bsdf->eval_f(connect_wi, z.w, BSDF_ALL);
      if(camera_bsdf_f.is_black()) { continue; }

      float light_connect_dir_pdf = z.bsdf->light_f_pdf(connect_wi, z.w, BSDF_ALL);
      float light_bwd_dir_pdf = y.bsdf->light_f_pdf(y.w, -connect_wi, BSDF_ALL);
      float camera_connect_dir_pdf = y.bsdf->camera_f_pdf(-connect_wi, y.w, BSDF_ALL);
      float camera_bwd_dir_pdf = z.bsdf->camera_f_pdf(z.w, connect_wi, BSDF_ALL);

      ShadowTest shadow;
      shadow.init_point_point(y.p, y.p_epsilon, z.p, z.p_epsilon);

      float dot_y = abs_dot(y.nn, connect_wi);
      float dot_z = abs_dot(z.nn, connect_wi);
      float dist_squared = length_squared(y.p - z.p);

      // equations (40), (41)
      float w_light = light_connect_dir_pdf * dot_y / dist_squared
        * (iter_state.mis_vm_weight + y.d_vcm + light_bwd_dir_pdf * y.d_vc);
      float w_camera = camera_connect_dir_pdf * dot_z / dist_squared
        * (iter_state.mis_vm_weight + z.d_vcm + camera_bwd_dir_pdf * z.d_vc);
      float weight = 1.f / (1.f + w_light + w_camera);

      Spectrum contrib = y.throughput * light_bsdf_f * camera_bsdf_f * z.throughput
        * (dot_y * dot_z / dist_squared);

      if(!contrib.is_black() && shadow.visible(*this->scene)) {
        this->store_debug_weighted_contrib(light_i + 2, bounces + 2, false,
            film_pos, contrib, weight);
        contrib_sum += contrib * weight;
      }
    }
    return contrib_sum;
  }

  Spectrum VcmRenderer::connect_to_light(const IterationState& iter_state,
      const LightPathState& light_path, const PathVertex& z,
      Vec2 film_pos, uint32_t bounces, Sampler& sampler) const
  {
    // connect vertex z to a new light vertex (VC, s = 1, t >= 2)
    Vector light_wi;
    Point light_p;
    Normal light_nn;
    float light_p_epsilon;
    float light_wi_pdf;
    ShadowTest shadow;
    Spectrum radiance = light_path.light->sample_pivot_radiance(z.p, z.p_epsilon,
        light_wi, light_p, light_nn, light_p_epsilon,
        light_wi_pdf, shadow, LightSample(sampler.rng));
    if(radiance.is_black()) { return Spectrum(0.f); }
    Spectrum bsdf_f = z.bsdf->eval_f(light_wi, z.w, BSDF_ALL);
    if(bsdf_f.is_black()) { return Spectrum(0.f); }

    float light_ray_pdf = light_path.light->ray_radiance_pdf(*this->scene,
        light_p, -light_wi, light_nn);
    float fwd_bsdf_dir_pdf = z.bsdf->light_f_pdf(light_wi, z.w, BSDF_ALL);
    float bwd_bsdf_dir_pdf = z.bsdf->camera_f_pdf(z.w, light_wi, BSDF_ALL);

    // equations (44), (45)
    float w_light, w_camera;
    if(!(light_path.light->flags & LIGHT_DELTA)) {
      w_light = fwd_bsdf_dir_pdf / (light_path.light_pick_pdf * light_wi_pdf);
    } else {
      w_light = 0.f;
    }
    if(light_path.light->flags & LIGHT_DISTANT) {
      w_camera = light_ray_pdf * abs_dot(z.nn, light_wi) / light_wi_pdf
        * (iter_state.mis_vm_weight + z.d_vcm + bwd_bsdf_dir_pdf * z.d_vc);
    } else {
      w_camera = light_ray_pdf * abs_dot(z.nn, light_wi)
        / (light_wi_pdf * abs_dot(light_nn, light_wi))
        * (iter_state.mis_vm_weight + z.d_vcm + bwd_bsdf_dir_pdf * z.d_vc);
    }
    float weight = 1.f / (1.f + w_light + w_camera);

    Spectrum contrib = z.throughput * bsdf_f * radiance 
      * (abs_dot(z.nn, light_wi) / (light_path.light_pick_pdf * light_wi_pdf));
    if(contrib.is_black() || !shadow.visible(*this->scene)) {
      return Spectrum(0.f);
    }

    this->store_debug_weighted_contrib(1, bounces + 2, false,
        film_pos, contrib, weight);
    return contrib * weight;
  }

  Spectrum VcmRenderer::connect_area_light(const PathVertex& zp,
      const PathVertex& z, const Light* area_light,
      Vec2 film_pos, uint32_t bounces) const
  {
    // "connect" vertex z as light (VC, s = 0, t >= 2, area light)
    assert(!(area_light->flags & LIGHT_DISTANT));
    assert(area_light->flags & LIGHT_AREA);
    if(bounces == 0) {
      // special case for (VC, s = 0, t = 2), no weighting is performed
      Spectrum radiance = area_light->eval_radiance(z.p, z.nn, zp.p);
      Spectrum contrib = z.throughput * radiance;
      if(!contrib.is_black()) {
        this->store_debug_weighted_contrib(0, bounces + 2, false,
            film_pos, contrib, 1.f);
      }
      return contrib;
    } else {
      Spectrum radiance = area_light->eval_radiance(z.p, z.nn, zp.p);
      if(radiance.is_black()) { return Spectrum(0.f); }

      float light_pick_pdf = this->light_distrib_pdfs.at(area_light);
      float light_ray_pdf = area_light->ray_radiance_pdf(*this->scene,
          z.p, normalize(zp.p - z.p), z.nn);
      float pivot_dir_pdf = area_light->pivot_radiance_pdf(-z.w, zp.p);

      // equations (42), (43)
      float w_camera = light_pick_pdf * (
        pivot_dir_pdf * abs_dot(z.nn, z.w) * z.d_vcm / length_squared(zp.p - z.p)
          + light_ray_pdf * z.d_vc);
      float weight = 1.f / (1.f + w_camera);

      Spectrum contrib = z.throughput * radiance;
      if(!contrib.is_black()) {
        this->store_debug_weighted_contrib(0, bounces + 2, false,
            film_pos, contrib, weight);
      }
      return contrib * weight;
    }
  }
  

  void VcmRenderer::init_debug_films() {
    if(this->debug_image_dir.empty()) { return; }
    for(uint32_t s = 0; s <= this->max_length; ++s) {
      for(uint32_t t = 1; t <= this->max_length; ++t) {
        for(bool is_vm: {true, false}) {
          bool skip = false;
          if(is_vm && (s < 2 || t < 2)) { skip = true; }
          if(is_vm && s + t > this->max_length + 1) { skip = true; }
          if(is_vm && s + t < this->min_length + 1) { skip = true; }
          if(!is_vm && s == 1 && t == 1) { skip = true; }
          if(!is_vm && s + t > this->max_length) { skip = true; }
          if(!is_vm && s + t < this->min_length) { skip = true; }
          if(skip) { continue; }

          for(bool is_weighted: {true, false}) {
            uint32_t key = this->debug_image_key(s, t, is_vm, is_weighted);
            auto name = std::to_string(s) + "_" + std::to_string(t) + 
              (is_vm ? "_vm" : "_vc") + (is_weighted ? "_w" : "_c");
            this->debug_names.insert(std::make_pair(key, name));
            this->debug_films.emplace(std::piecewise_construct,
                std::forward_as_tuple(key),
                std::forward_as_tuple(
                  this->film->res.x, this->film->res.y, this->film->filter));
          }
        }
      }
    }
  }

  void VcmRenderer::save_debug_films() {
    if(this->debug_image_dir.empty()) { return; }
    for(const auto& pair: this->debug_names) {
      uint32_t key = pair.first;
      auto& name = pair.second;
      auto& film = this->debug_films.at(key);
      film.splat_scale = 1.f / float(this->iteration_count);
      auto image = film.to_image<PixelRgbFloat>();

      auto path = this->debug_image_dir + "/vcm_" + name + ".hdr";
      FILE* output = std::fopen(path.c_str(), "w");
      assert(output != nullptr);
      if(output != nullptr) {
        write_image_rgbe(output, image);
        std::fclose(output);
      }
    }
  }

  void VcmRenderer::store_debug_weighted_contrib(uint32_t s, uint32_t t, bool is_vm,
      Vec2 film_pos, const Spectrum& contrib, float weight) const
  {
    if(!this->debug_image_dir.empty()) {
      this->store_debug_contrib(s, t, is_vm, false, film_pos, contrib);
      this->store_debug_contrib(s, t, is_vm, true, film_pos, contrib * weight);
    }
  }

  void VcmRenderer::store_debug_contrib(uint32_t s, uint32_t t, bool is_vm,
      bool is_weighted, Vec2 film_pos, const Spectrum& contrib) const
  {
    if(this->debug_image_dir.empty()) { return; }
    uint32_t key = this->debug_image_key(s, t, is_vm, is_weighted);
    auto iter = this->debug_films.find(key);
    assert(iter != this->debug_films.end());
    if(iter != this->debug_films.end()) {
      iter->second.add_splat(film_pos, contrib);
    }
  }

  uint32_t VcmRenderer::debug_image_key(uint32_t s, uint32_t t,
      bool is_vm, bool is_weighted) const
  {
    return is_weighted | (is_vm << 1) | (t << 2) | (s << 6);
  }
}

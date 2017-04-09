#include "dort/vcm_renderer.hpp"

namespace dort {
  void VcmRenderer::render(CtxG& ctx, Progress& progress) {
    this->light_distrib = compute_light_distrib(scene);
    this->background_light_distrib = compute_light_distrib(scene, true);
    for(uint32_t i = 0; i < scene.lights.size(); ++i) {
      this->light_distrib_pdfs.insert(std::make_pair(
          scene.lights.at(i).get(), this->light_distrib.pdf(i)));
    }

    std::vector<Photon> photons;
    for(uint32_t i = 0; i < this->iteration_count; ++i) {
      this->iteration(i, photons);
      progress.set_percent_done(float(i + 1) / float(this->iteration_count));
      if(progress.is_cancelled()) { break; }
    }
  }

  void VcmRenderer::iteration(uint32_t idx, std::vector<Photon> photons) {
    float radius = this->initial_radius
      * pow(float(idx + 1), 0.5f * (this->alpha - 1.f));
    uint32_t path_count = this->film->x_res * this->film->y_res;
    float eta_vcm = PI * square(radius) * float(path_count);

    IterationState iter_state;
    iter_state.idx = idx;
    iter_state.path_count = path_count;
    iter_state.radius = radius;
    iter_state.mis_vm_weight = this->use_vm ? eta_vcm : 0.f;
    iter_state.mis_vc_weight = this->use_vc ? 1.f / eta_vcm : 0.f;
    iter_state.vm_normalization = 1.f / eta_vcm;
    iter_state.photon_tree = KdTree<PhotonKdTraits>(std::move(photons));

    if(idx == 0) {
      assert(photons.empty());
    } else if(idx == 1) {
      iter_state.vm_normalization *= 2.f;
    }

    this->iteration(iter_state, *this->sampler);
  }

  void VcmRenderer::iteration(const IterationState& iter_state, Sampler& sampler) {
    std::vector<Photon> photons;
    std::vector<PathVertex> light_vertices;
    for(uint32_t path_idx = 0; path_idx < path_count; ++path_idx) {
      LightPathState light_path = this->light_walk(iter_state, light_vertices, sampler);
      uint32_t film_y = path_idx / this->film->x_res;
      uint32_t film_x = path_idx % this->film->x_res;
      Vec2 film_pos = Vec2(film_x, film_y) + sampler.random_2d();
      Spectrum contrib = this->camera_walk(iter_state, light_path, light_vertices, film_pos, sampler);
      this->film->add_sample(film_pos, contrib);
      light_vertices.clear();

    }

  }

  LightPathState VcmRenderer::light_walk(const IterationState& iter_state,
      std::vector<PathVertex>& light_vertices, Sampler& sampler)
  {
    // pick a light and sample the first ray
    uint32_t light_i = this->light_distrib.sample(rng.uniform_float());
    float light_pick_pdf = this->light_distrib.pdf(light_i);
    const Light& light = *scene.lights.at(light_i);

    Ray light_ray;
    Normal light_nn;
    float light_pos_pdf, light_dir_pdf;
    Spectrum light_radiance = light.sample_ray_radiance(*this->scene,
        light_ray, light_nn, light_pos_pdf, light_dir_pdf, LightRaySample(sampler.rng));

    float light_ray_pdf = light_pos_pdf * light_dir_pdf * light_pick_pdf;
    if(light_ray_pdf == 0.f) { continue; }

    Spectrum throughput = light_radiance / light_ray_pdf;
    float fwd_bsdf_dir_pdf = SIGNALING_NAN;
    std::vector<PathVertex> light_vertices;

    for(uint32_t bounces = 0;; ++bounces) {
      Intersection isect;
      if(!this->scene->intersect(light_ray, isect)) {
        break;
      }

      PathVertex y;
      y.p = isect.world_diff_geom.p;
      y.p_epsilon = isect.ray_epsilon;
      y.w = -light_ray.dir;
      y.nn = isect.world_diff_geom.nn;
      y.throughput = throughput;
      y.bsdf = isect.get_bsdf();

      // compute the MIS quantities
      if(bounces == 0) {
        float pivot_dir_pdf = light.pivot_radiance_pdf(y.w, y.p);
        // equations (31), (32), (33)
        y.d_vcm = pivot_dir_pdf * abs_dot(light_nn, y.w) /
          (light_dir_pdf * light_pos_pdf * abs_dot(y.nn, y.w));
        y.d_vc = abs_dot(light_nn, y.w) /
          (light_ray_pdf * abs_dot(y.nn, y.w));
        y.d_vm = y.d_vc * iter_state.mis_vm_weight;
      } else {
        const auto& yp = light_vertices.at(bounces);
        float bwd_bsdf_dir_pdf = yp.bsdf->light_f_pdf(yp.w, -y.w, BSDF_ALL);
        // equations (34), (35), (36)
        y.d_vcm = length_squared(yp.p - y.p) /
          (fwd_bsdf_dir_pdf * abs_dot(y.nn * y.w));
        y.d_vc = abs_dot(yp.nn, y.w) / (abs_dot(y.nn, y.w) * fwd_bsdf_dir_pdf)
          * (iter_state.mis_vm_weight + yp.d_vcm + bwd_bsdf_dir_pdf * yp.d_vc);
        y.d_vm = abs_dot(yp.nn, y.w) / (abs_dot(y.nn, y.w) * fwd_bsdf_dir_pdf)
          * (1.f + yp.d_vcm * iter_state.mis_vc_weight + bwd_bsdf_dir_pdf * yp.d_vm);
      }

      if(bounces + 2 <= this->max_length && bounces + 2 >= this->min_length) {
        // connect vertex y to camera (VC, s >= 2, t = 1)
        Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
        float path_count = float(iter_state.path_count);

        Point camera_p;
        Vec2 film_pos;
        float camera_p_pdf;
        ShadowTest shadow;
        Spectrum importance = this->camera->sample_pivot_importance(film_res,
            y.p, y.p_epsilon, camera_p, film_pos, camera_p_pdf,
            shadow, CameraSample(sampler.rng));

        Vector camera_wi = normalize(y.p - camera_p);
        float camera_ray_pdf = this->camera->ray_importance_pdf(film_res,
          camera_p, camera_wi);

        Spectrum bsdf_f = y.bsdf->eval_f(y.w, -camera_wi, BSDF_ALL);
        float bwd_bsdf_dir_pdf = y.bsdf->light_f_pdf(y.w, -camera_wi, BSDF_ALL);

        // equation (46), (37)
        float w_light = camera_ray_pdf * abs_dot(y.nn, camera_wi)
          / (camera_p_pdf * length_squared(y.p - camera_p) * path_count)
          * (iter_state.mis_vm_weight + y.d_vcm + bwd_bsdf_dir_pdf * y.d_vc);
        float weight = 1.f / (1.f + w_light);

        Spectrum contrib = y.throughput * bsdf_f * importance / (camera_p_pdf * path_count);
        if(!contrib.is_black() && shadow.is_visible(*this->scene)) {
          this->film->add_splat(film_pos, contrib * weight);
        }
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

      light_ray = Ray(y.p, bsdf_wo, y.p_epsilon);
      fwd_bsdf_dir_pdf = bsdf_wo_pdf;
      throughput *= bsdf_f * (abs_dot(y.nn, bsdf_wo) / bsdf_wo_pdf);
      light_vertices.push_back(std::move(y));
    }

    LightPathState path;
    path.light = light;
    path.light_pick_pdf = light_pick_pdf;
    return path;
  }

  void VcmRenderer::camera_walk(const IterationState& iter_state,
      const LightPathState& light_path,
      const std::vector<PathVertex>& light_vertices,
      Vec2 film_pos, Sampler& sampler)
  {
    Vec2 film_res(float(this->film->x_res), float(this->film->y_res));
    Ray camera_ray;
    float camera_pos_pdf, camera_dir_pdf;
    Spectrum camera_importance = this->camera->sample_ray_importance(film_res, film_pos,
        camera_ray, camera_pos_pdf, camera_dir_pdf, CameraSample(sampler.rng));

    float camera_ray_pdf = camera_pos_pdf * camera_dir_pdf;
    if(camera_ray_pdf == 0.f) { return Spectrum(0.f); }

    Spectrum throughput = camera_importance / camera_ray_pdf;
    float fwd_bsdf_dir_pdf = SIGNALING_NAN;
    PathVertex zp;
    Spectrum film_contrib(0.f);

    for(uint32_t bounces = 0;; ++bounces) {
      Intersection isect;
      if(!this->scene->intersect(camera_ray, isect)) {
        // TODO: handle background lights
        break;
      }

      PathVertex z;
      z.p = isect.world_diff_geom.p;
      z.p_epsilon = isect.ray_epsilon;
      z.w = -camera_ray.dir;
      z.nn = isect.world_diff_geom.nn;
      z.throughput = throughput;
      z.bsdf = isect.get_bsdf();

      // compute the MIS quantities
      if(bounces == 0) {
        // equations (31), (32), (33)
        float pivot_area_pdf = this->camera->pivot_importance_pdf(film_res, camera_ray.orig, z.p);
        z.d_vcm = pivot_area_pdf * float(iter_state.path_count)
          * length_squared(z.p - camera_ray.orig)
          / (camera_ray_pdf * abs_dot(z.nn, z.w));
        z.d_vc = 0.f;
        z.d_vm = 0.f;
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

      if(bounces + 2 <= this->max_length) {
        // merge vertex z with photons
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

            float bwd_light_dir_pdf = z.bsdf->light_f_pdf(photon.wi, -z.w, BSDF_ALL);
            float bwd_camera_dir_pdf = z.bsdf->camera_f_pdf(z.w, photon.wi, BSDF_ALL);

            // equations (38), (39)
            float w_light = photon.d_vcm * iter_state.mis_vc_weight
              + bwd_light_dir_pdf * photon.d_vm;
            float w_camera = z.d_vcm * iter_state.mis_vc_weight
              + bwd_camera_dir_pdf * z.d_vm;
            float weight = 1.f / (w_light + w_camera + 1.f);

            photon_sum += bsdf_f * photon.throughput * weight;
            return radius_square;
          });

        film_contrib += photon_sum * z.throughput * iter_state.vm_normalization;
      }

      if(bounces + 3 <= this->max_length) {
        // connect vertex z to light vertices
        for(uint32_t light_i = 0; light_i < light_vertices.size(); ++light_i) {
          if(bounces + light_i + 3 < this->min_length) { continue; }
          if(bounces + light_i + 3 > this->max_length) { break; }
          const PathVertex& y = light_vertices.at(light_i);

          Vector connect_wi = normalize(y.p - z.p);
          float light_connect_dir_pdf = z.bsdf->light_f_pdf(connect_wi, z.w, BSDF_ALL);
          float light_bwd_dir_pdf = y.bsdf->light_f_pdf(y.w, -connect_wi, BSDF_ALL);
          float camera_connect_dir_pdf = y.bsdf->camera_f_pdf(-connect_wi, y.w, BSDF_ALL);
          float camera_bwd_dir_pdf = z.bsdf->camera_f_pdf(z.w, connect_wi, BSDF_ALL);

          Spectrum light_bsdf_f = y.bsdf->eval_f(y.w, -connect_wi, BSDF_ALL);
          Spectrum camera_bsdf_f = z.bsdf->eval_f(connect_wi, z.w, BSDF_ALL);

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

          float geom = abs_dot(y.nn, connect_wi) * abs_dot(z.nn, connect_wi)
          Spectrum contrib = y.throughput * light_bsdf_f * camera_bsdf_f * z.throughput
            * (dot_y * dot_z / dist_squared);

          if(!contrib.is_black() && shadow.is_visible()) {
            film_contrib += contrib * weight;
          }
        }
      }

      if(bounces + 2 <= this->max_length && bounces + 2 >= this->min_length) {
        // connect vertex z to a new light vertex
        Vector light_wi;
        Point light_p;
        Normal light_nn;
        float light_p_epsilon;
        float light_wi_pdf;
        ShadowTest shadow;
        Spectrum radiance = light_path.light->sample_pivot_radiance(z.p, z.p_epsilon,
            light_wi, light_p, light_nn, light_p_epsilon,
            light_wi_pdf, shadow, LightSample(sampler.rng));
        Spectrum bsdf_f = z.bsdf->eval_f(light_wi, z.w, BSDF_ALL);

        ShadowTest shadow;
        shadow.init_point_point(light_p, light_p_epsilon, z.p, z.p_epsilon);

        float light_ray_pdf = light_path.light->ray_radiance_pdf(*this->scene,
            light_p, -light_wi, light_nn);
        float fwd_bsdf_dir_pdf = z.bsdf->light_f_pdf(light_wi, z.w, BSDF_ALL);
        float bwd_bsdf_dir_pdf = z.bsdf->camera_f_pdf(z.w, light_wi, BSDF_ALL);

        // equations (44), (45)
        float w_light = fwd_bsdf_dir_pdf / (light_path.light_pick_pdf * light_wi_pdf);
        float w_camera = light_ray_pdf * abs_dot(z.nn, light_wi)
          / (light_wi_pdf * abs_dot(light_nn, light_wi))
          * (iter_state.mis_vm_weight + y.d_vcm + bwd_bsdf_dir_pdf * z.d_vc);
        float weight = 1.f / (1.f + w_light + w_camera);

        Spectrum contrib = z.throughput * bsdf_f * radiance / light_wi_pdf;
        if(!contrib.is_black() && shadow.is_visible(*this->scene)) {
          film_contrib += contrib * weight;
        }
      }

      if(bounces + 1 <= this->max_length && bounces + 1 >= this->min_length) {
        // "connect" vertex z as light (if z is on area light)
        const Light* area_light = isect.get_area_light();
        if(area_light != nullptr) {
          float light_pick_pdf = this->light_distrib_pdfs.at(area_light);
          float light_ray_pdf = area_light->ray_radiance_pdf(*this->scene,
              z.p, normalize(zp.p - z.p), z.nn);
          float pivot_dir_pdf = area_light->pivot_radiance_pdf(-z.w, zp.p);

          Spectrum radiance = area_light->eval_radiance(z.p, z.nn, zp.p);

          // equations (42), (43)
          float w_camera = light_pick_pdf * (
            pivot_dir_pdf * abs_dot(z.nn, z.w) * z.d_vcm / length_squared(zp.p - z.p)
             + light_ray_pdf * z.d_vc);
          float weight = 1.f / (1.f + w_camera);

          Spectrum contrib = z.throughput * radiance;
          if(!contrib.is_black()) {
            film_contrib += contrib * weight;
          }
        }
      }

      // the path is too long to be ever accepted
      if(bounces + 1 > this->max_length) { break; }

      // bounce
      Vector bsdf_wi;
      float bsdf_wi_pdf;
      BxdfFlags bsdf_flags;
      Spectrum bsdf_f = z.bsdf->sample_light_f(z.w, BSDF_ALL,
          bsdf_wi, bsdf_wi_pdf, bsdf_flags, BsdfSample(sampler.rng));

      camera_ray = Ray(z.p, bsdf_wi, isect.ray_epsilon);
      fwd_bsdf_dir_pdf = bsdf_wi_pdf;
      throughput *= bsdf_f * (abs_dot(z.nn, bsdf_wi) / bsdf_wi_pdf);
      zp = std::move(z);
    }

    return film_contrib;
  }
}

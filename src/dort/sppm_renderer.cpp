#if 0
#include <mutex>
#include <shared_mutex>
#include "dort/camera.hpp"
#include "dort/ctx.hpp"
#include "dort/film.hpp"
#include "dort/lighting.hpp"
#include "dort/low_discrepancy.hpp"
#include "dort/primitive.hpp"
#include "dort/sppm_renderer.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  void SppmRenderer::render(CtxG& ctx, Progress& progress) {
    this->light_distrib = compute_light_distrib(*this->scene);
    this->pixel_pos_idx = this->sampler->request_sample_2d();

    auto mode = this->parallel_mode;
    if(mode == ParallelMode::Automatic) {
      if(ctx.pool->thread_count() <= this->iteration_count) {
        mode = ParallelMode::ParallelIterations;
      } else {
        mode = ParallelMode::SerialIterations;
      }
    }

    std::vector<float> radii(this->iteration_count);
    {
      float radius = this->initial_radius;
      for(uint32_t i = 0; i < this->iteration_count; ++i) {
        radii.at(i) = radius;
        radius *= sqrt((float(i) + this->alpha) / (float(i) + 1.f));
      }
    }

    if(mode == ParallelMode::ParallelIterations) {
      std::vector<std::shared_ptr<Sampler>> samplers;
      for(uint32_t i = 0; i < this->iteration_count; ++i) {
        samplers.push_back(sampler->split());
      }

      std::mutex film_mutex;
      std::atomic<uint32_t> iterations_done(0);
      fork_join(*ctx.pool, this->iteration_count, [&](uint32_t i) {
        if(progress.is_cancelled()) { return; }

        Film local_film(this->film->x_res, this->film->y_res, this->film->filter);
        this->iteration_serial(local_film, *samplers.at(i), radii.at(i));
        {
          std::unique_lock<std::mutex> film_lock(film_mutex);
          this->film->add_tile(Vec2i(0, 0), local_film);
        }

        uint32_t done = iterations_done.fetch_add(1) + 1;
        progress.set_percent_done(float(done) / float(this->iteration_count));
      });
    } else {
      for(uint32_t i = 0; i < this->iteration_count; ++i) {
        this->iteration_parallel(ctx, *this->film, *this->sampler, radii.at(i));
        progress.set_percent_done(float(i) / float(this->iteration_count));
        if(progress.is_cancelled()) { return; }
      }
    }
  }

  void SppmRenderer::iteration_serial(Film& film, Sampler& sampler, float radius) const {
    PhotonMap photon_map = this->compute_photon_map_serial(sampler.rng);
    Recti film_rect(Vec2i(0, 0), Vec2i(film.x_res, film.y_res));
    this->gather_tile(film, film, sampler, film_rect, film_rect, photon_map, radius);
  }

  void SppmRenderer::iteration_parallel(CtxG& ctx, Film& film,
      Sampler& sampler, float radius) const 
  {
    PhotonMap photon_map = this->compute_photon_map_parallel(ctx, sampler.rng);

    Vec2i layout_tiles = Renderer::layout_tiles(ctx, film, sampler);
    Vec2 tile_size = Vec2(float(this->film->x_res), float(this->film->y_res)) /
      Vec2(float(layout_tiles.x), float(layout_tiles.y));
    uint32_t job_count = layout_tiles.x * layout_tiles.y;

    std::vector<std::shared_ptr<Sampler>> samplers;
    for(uint32_t i = 0; i < job_count; ++i) {
      samplers.push_back(this->sampler->split());
    }

    std::mutex film_mutex;
    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      uint32_t tile_x = job_i % layout_tiles.x;
      uint32_t tile_y = job_i / layout_tiles.x;
      Vec2 corner_0f = tile_size * Vec2(float(tile_x), float(tile_y));
      Vec2 corner_1f = corner_0f + tile_size;
      Recti tile_rect(floor_vec2i(corner_0f), floor_vec2i(corner_1f));

      Vec2 margin = film.filter.radius;
      Recti film_rect(floor_vec2i(corner_0f - margin), ceil_vec2i(corner_1f + margin));
      Vec2i film_size = film_rect.p_max - film_rect.p_min;
      Film tile_film(film_size.x, film_size.y, film.filter);
      this->gather_tile(film, tile_film, *samplers.at(job_i),
        tile_rect, film_rect, photon_map, radius);

      std::unique_lock<std::mutex> film_lock(film_mutex);
      film.add_tile(film_rect.p_min, tile_film);
    });
  }

  void SppmRenderer::gather_tile(const Film& film, Film& tile_film, Sampler& sampler,
      Recti tile_rect, Recti tile_film_rect,
      const PhotonMap& photon_map, float radius) const
  {
    Vec2 film_res = Vec2(float(film.x_res), float(film.y_res));
    const Camera& camera = *this->camera;

    for(int32_t y = tile_rect.p_min.y; y < tile_rect.p_max.y; ++y) {
      for(int32_t x = tile_rect.p_min.x; x < tile_rect.p_max.x; ++x) {
        sampler.start_pixel();
        for(uint32_t s = 0; s < sampler.samples_per_pixel; ++s) {
          sampler.start_pixel_sample();

          Vec2 pixel_pos = sampler.get_sample_2d(this->pixel_pos_idx);
          Vec2 film_pos = Vec2(float(x), float(y)) + pixel_pos;
          Ray camera_ray;
          float camera_pos_pdf, camera_dir_pdf;
          Spectrum importance = camera.sample_ray_importance(film_res, film_pos,
              camera_ray, camera_pos_pdf, camera_dir_pdf, CameraSample(sampler.rng));
          Spectrum radiance = this->gather_ray(camera_ray, sampler, photon_map, radius);
          float camera_ray_pdf = camera_pos_pdf * camera_dir_pdf;

          Vec2 tile_film_pos = film_pos -
            Vec2(float(tile_film_rect.p_min.x), float(tile_film_rect.p_min.y));
          tile_film.add_sample(tile_film_pos, importance * radiance / camera_ray_pdf);
        }
      }
    }
  }

  Spectrum SppmRenderer::gather_ray(Ray ray, Sampler& sampler,
      const PhotonMap& photon_map, float radius) const
  {
    Spectrum radiance(0.f);
    Spectrum weight(1.f);

    for(uint32_t depth = 0; depth < this->max_depth; ++depth) {
      Intersection isect;
      if(!this->scene->intersect(ray, isect)) {
        for(const auto& light: this->scene->background_lights) {
          radiance += weight * light->background_radiance(ray);
        }
        break;
      }

      Spectrum emitted = isect.eval_radiance(ray.orig);
      assert(is_finite(emitted) && is_nonnegative(emitted));
      radiance += weight * emitted;

      LightingGeom geom;
      geom.p = isect.world_diff_geom.p;
      geom.nn = isect.world_diff_geom.nn;
      geom.wo_camera = normalize(-ray.dir);
      geom.ray_epsilon = isect.ray_epsilon;

      std::unique_ptr<Bsdf> bsdf = isect.get_bsdf();
      BxdfFlags NON_DIFFUSE = BSDF_ALL & ~BSDF_DIFFUSE;
      if(depth + 1 < this->max_depth && bsdf->num_bxdfs(NON_DIFFUSE) > 0) {
        Vector bsdf_wi;
        float wi_bsdf_pdf;
        BxdfFlags bsdf_flags;
        Spectrum bsdf_f = bsdf->sample_light_f(geom.wo_camera, NON_DIFFUSE,
            bsdf_wi, wi_bsdf_pdf, bsdf_flags, BsdfSample(sampler.rng));
        if(wi_bsdf_pdf == 0.f || bsdf_f.is_black()) {
          break;
        }

        ray = Ray(geom.p, bsdf_wi, geom.ray_epsilon);
        weight *= bsdf_f * (abs_dot(bsdf_wi, geom.nn) / wi_bsdf_pdf);
      } else {
        Spectrum photon_radiance = photon_map.estimate_radiance(
            geom.p, geom.nn, geom.wo_camera, *bsdf, radius);
        Spectrum direct_radiance = uniform_sample_one_light(
            *this->scene, geom, *bsdf, sampler);
        assert(is_finite(photon_radiance) && is_nonnegative(photon_radiance));
        assert(is_finite(direct_radiance) && is_nonnegative(direct_radiance));
        radiance += weight * (photon_radiance + direct_radiance);
        break;
      }
    }

    return radiance;
  }

  PhotonMap SppmRenderer::compute_photon_map_serial(Rng& rng) const {
    std::vector<Photon> photons;
    this->shoot_photons(photons, this->photon_path_count, rng);
    return PhotonMap(std::move(photons), this->photon_path_count);
  }

  PhotonMap SppmRenderer::compute_photon_map_parallel(CtxG& ctx, Rng& rng) const {
    uint32_t target_path_count = this->photon_path_count;
    uint32_t job_count = max(1u, ctx.pool->thread_count());
    uint32_t block_size = clamp(target_path_count / (4 * job_count), 128u, 1024u);

    std::vector<Rng> rngs;
    for(uint32_t i = 0; i < job_count; ++i) {
      rngs.push_back(rng.split());
    }

    std::vector<Photon> photons;
    std::atomic<uint32_t> photon_count(0);
    std::atomic<uint32_t> path_count(0);
    std::shared_timed_mutex photons_mutex;

    auto ensure_photons_size = [&](uint32_t min_size) {
      std::unique_lock<std::shared_timed_mutex> resize_lock(photons_mutex);
      if(min_size <= photons.size()) {
        return;
      }
      photons.resize(min_size);
      photons.resize(photons.capacity());
    };

    auto add_photons = [&](const std::vector<Photon>& block) {
      std::shared_lock<std::shared_timed_mutex> write_lock(photons_mutex);
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

    fork_join(*ctx.pool, job_count, [&](uint32_t job_i) {
      Rng rng(std::move(rngs.at(job_i)));

      std::vector<Photon> block_photons;
      for(;;) {
        uint32_t prev_path_count = path_count.fetch_add(block_size);
        if(prev_path_count >= target_path_count) {
          break;
        }

        block_photons.clear();
        this->shoot_photons(block_photons,
          min(block_size, target_path_count - prev_path_count), rng);
        add_photons(block_photons);
      }
    });

    photons.resize(photon_count.load());
    return PhotonMap(std::move(photons), target_path_count);
  }

  void SppmRenderer::shoot_photons(std::vector<Photon>& photons,
      uint32_t count, Rng& rng) const
  {
    std::vector<float> light_idx_samples(low_discrepancy_1d(1, count, rng));
    std::vector<Vec2> light_pos_samples(low_discrepancy_2d(1, count, rng));
    std::vector<Vec2> light_dir_samples(low_discrepancy_2d(1, count, rng));

    for(uint32_t i = 0; i < count; ++i) {
      uint32_t light_i = this->light_distrib.sample(light_idx_samples.at(i));
      float light_pdf = this->light_distrib.pdf(light_i);
      const Light& light = *this->scene->lights.at(light_i);
      LightRaySample light_sample(light_pos_samples.at(i), light_dir_samples.at(i));

      Ray ray;
      Normal prev_nn;
      float light_pos_pdf, light_dir_pdf;
      Spectrum light_radiance = light.sample_ray_radiance(
          *this->scene, ray, prev_nn, light_pos_pdf, light_dir_pdf, light_sample);
      float light_ray_pdf = light_pos_pdf * light_dir_pdf;
      if(light_radiance.is_black() || light_pdf == 0.f || light_ray_pdf == 0.f) {
        continue;
      }

      Spectrum path_power = light_radiance * (abs_dot(ray.dir, prev_nn) 
        / (light_ray_pdf * light_pdf));
      for(uint32_t depth = 0; depth < this->max_photon_depth; ++depth) {
        Intersection isect;
        if(!this->scene->intersect(ray, isect)) {
          break;
        }
        auto bsdf = isect.get_bsdf();
        bool has_non_delta = bsdf->num_bxdfs(BSDF_DELTA) < bsdf->num_bxdfs();

        if(depth > 0 && has_non_delta) {
          assert(is_finite(path_power));
          assert(is_nonnegative(path_power));

          Photon photon;
          photon.power = path_power;
          photon.p = isect.world_diff_geom.p;
          photon.wi = -ray.dir;
          photon.nn = isect.world_diff_geom.nn;
          photons.push_back(photon);
        }

        Vector bounce_wo;
        float bounce_pdf;
        BxdfFlags bounce_flags;
        Spectrum bounce_f = bsdf->sample_camera_f(-ray.dir, BSDF_ALL,
            bounce_wo, bounce_pdf, bounce_flags, BsdfSample(rng));
        Spectrum bounce_contrib = bounce_f * (abs_dot(bounce_wo,
              isect.world_diff_geom.nn) / bounce_pdf);
        if(bounce_pdf == 0.f || bounce_contrib.is_black()) {
          break;
        }

        path_power *= bounce_contrib;

        if(depth > 0) {
          float survive_prob = min(0.95f, bounce_contrib.average());
          if(rng.uniform_float() > survive_prob) {
            break;
          }
          path_power /= survive_prob;
        }

        ray = Ray(isect.world_diff_geom.p, bounce_wo, isect.ray_epsilon);
      }
    }
  }
}
#endif

#include "dort/light.hpp"
#include "dort/primitive.hpp"

namespace dort {
  LightSample::LightSample(Rng& rng) {
    this->uv_pos = Vec2(rng.uniform_float(), rng.uniform_float());
  }

  LightSample::LightSample(Sampler& sampler, const LightSamplesIdxs& idxs, uint32_t n) {
    this->uv_pos = sampler.get_array_2d(idxs.uv_pos_idx).at(n);
  }

  LightSamplesIdxs LightSample::request(Sampler& sampler, uint32_t count) {
    LightSamplesIdxs idxs;
    idxs.uv_pos_idx = sampler.request_array_2d(count);
    idxs.count = count;
    return idxs;
  }

  LightRaySample::LightRaySample(Rng& rng) {
    this->uv_pos = Vec2(rng.uniform_float(), rng.uniform_float());
    this->uv_dir = Vec2(rng.uniform_float(), rng.uniform_float());
  }

  LightRaySample::LightRaySample(Sampler& sampler,
      const LightRaySamplesIdxs& idxs, uint32_t n)
  {
    this->uv_pos = sampler.get_array_2d(idxs.uv_pos_idx).at(n);
    this->uv_dir = sampler.get_array_2d(idxs.uv_dir_idx).at(n);
  }

  LightRaySamplesIdxs LightRaySample::request(Sampler& sampler, uint32_t count) {
    LightRaySamplesIdxs idxs;
    idxs.uv_pos_idx = sampler.request_array_2d(count);
    idxs.uv_dir_idx = sampler.request_array_2d(count);
    idxs.count = count;
    return idxs;
  }

  void ShadowTest::init_point_point(const Point& p1, float epsilon_1,
      const Point& p2, float epsilon_2)
  {
    float distance = length(p2 - p1);
    this->ray.orig = p1;
    this->ray.dir = (p2 - p1) / distance;
    this->ray.t_min = epsilon_1;
    this->ray.t_max = distance - epsilon_2;
    this->invisible = false;
  }

  void ShadowTest::init_point_dir(const Point& pt, float epsilon,
      const Vector& dir)
  {
    this->ray.orig = pt;
    this->ray.dir = normalize(dir);
    this->ray.t_min = epsilon;
    this->ray.t_max = INFINITY;
    this->invisible = false;
  }

  void ShadowTest::init_invisible() {
    this->invisible = true;
  }

  bool ShadowTest::visible(const Scene& scene) const {
    return !this->invisible && !scene.intersect_p(this->ray);
  }
  
  Spectrum Light::background_radiance(const Ray&) const {
    return Spectrum();
  }
}

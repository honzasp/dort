#include "dort/camera.hpp"
#include "dort/light.hpp"

namespace dort {
  CameraSample::CameraSample(Rng& rng) {
    this->uv_lens = Vec2(rng.uniform_float(), rng.uniform_float());
  }

  CameraSample::CameraSample(Sampler& sampler, const CameraSamplesIdxs& idxs, uint32_t n) {
    this->uv_lens = sampler.get_array_2d(idxs.uv_lens_idx).at(n);
  }

  CameraSamplesIdxs CameraSample::request(Sampler& sampler, uint32_t count) {
    CameraSamplesIdxs idxs;
    idxs.uv_lens_idx = sampler.request_array_2d(count);
    idxs.count = count;
    return idxs;
  }
}

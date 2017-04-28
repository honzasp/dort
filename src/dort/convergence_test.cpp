#include <atomic>
#include <mutex>
#include "dort/convergence_test.hpp"
#include "dort/ctx.hpp"
#include "dort/image.hpp"
#include "dort/rect_i.hpp"
#include "dort/thread_pool.hpp"

namespace dort {
  std::string test_convergence(CtxG& ctx,
      const Image<PixelRgbFloat>& reference,
      const Image<PixelRgbFloat>& tested,
      int32_t min_tile_size, float variation, float bias, float p_value)
  {
    assert(reference.res == tested.res);
    Vec2i res(reference.res);

    auto compute_mean = [](const Image<PixelRgbFloat>& image, Recti tile) {
      Spectrum sum(0.f);
      int32_t count = 0;
      for(int32_t y = tile.p_min.y; y < tile.p_max.y; ++y) {
        for(int32_t x = tile.p_min.x; x < tile.p_max.x; ++x) {
          sum += PixelRgbFloat::to_rgb(image.get_pixel(x, y));
          count += 1;
        }
      }
      return sum / float(count);
    };

    uint32_t max_depth = 0;
    while((min(res.x, res.y) >> (max_depth + 1)) >= min_tile_size) {
      max_depth += 1;
    }
    float test_count = float(((1 << (2*max_depth + 2)) - 1) / 3);
    float test_p_value = 1.f - pow(1.f - p_value, 1.f / test_count);
    float inv_phi = normal_cdf_inverse(test_p_value * 0.5f);
    Spectrum ref_mean = compute_mean(reference, Recti(Vec2i(0, 0), res));

    auto test_tile = [&](Recti tile_res) -> std::string {
      Spectrum tile_ref_mean = compute_mean(reference, tile_res);
      Spectrum tile_tested_mean = compute_mean(tested, tile_res);
      float mean_diff = (tile_tested_mean - tile_ref_mean).max();

      float stddev_global = (ref_mean * variation).max();
      float stddev_local = (tile_ref_mean * variation).max();
      float stddev = max(stddev_global, stddev_local);

      float bias_global = (ref_mean * bias).max();
      float bias_local = (tile_ref_mean * bias).max();
      float bias = min(bias_global, bias_local);

      float count = float(tile_res.p_max.x - tile_res.p_min.x)
        * float(tile_res.p_max.y - tile_res.p_min.y);
      float max_diff = -stddev / sqrt(count) * inv_phi + bias;
      assert(max_diff >= 0.f);

      if(abs(mean_diff) > max_diff) {
        return "tile (" + std::to_string(tile_res.p_min.x) + "," +
          std::to_string(tile_res.p_min.y) + "):(" +
          std::to_string(tile_res.p_max.x) + "," +
          std::to_string(tile_res.p_max.y) + ") has diff " +
          std::to_string(mean_diff) + ", but threshold is " +
          std::to_string(max_diff);
      }
      return "";
    };

    auto test_depth = [&](uint32_t depth) -> std::string {
      int32_t tile_size = min(res.x, res.y) >> depth;
      int32_t tiles_x = (res.x + tile_size - 1) / tile_size;
      int32_t tiles_y = (res.y + tile_size - 1) / tile_size;
      for(int32_t tile_y = 0; tile_y < tiles_y; ++tile_y) {
        for(int32_t tile_x = 0; tile_x < tiles_x; ++tile_x) {
          auto error = test_tile(Recti(tile_x * tile_size, tile_y * tile_size,
              min((tile_x + 1) * tile_size, res.x),
              min((tile_y + 1) * tile_size, res.y)));
          if(!error.empty()) { return error; }
        }
      }
      return "";
    };

    std::mutex error_mutex;
    std::string error;
    // ensure that we always report the first error
    std::atomic<uint32_t> error_depth(-1u);

    fork_join(*ctx.pool, max_depth + 1, [&](uint32_t depth) {
      if(error_depth.load() < depth) { return; }
      auto local_error = test_depth(depth);
      if(local_error.empty()) { return; }

      std::unique_lock<std::mutex> error_lock(error_mutex);
      if(depth < error_depth.load()) {
        error = local_error;
        error_depth.store(depth);
      }
    });

    return error;
  }
}

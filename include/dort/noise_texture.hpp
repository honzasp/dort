#pragma once
#include <vector>
#include "dort/texture.hpp"
#include "dort/vec_3.hpp"
#include "dort/vec_3i.hpp"

namespace dort {
  struct NoiseLayer {
    float scale;
    float weight;
  };

  class NoiseTexture: public Texture<float> {
    std::shared_ptr<TextureMap3d> texture_map_or_null;
    std::vector<NoiseLayer> layers;
  public:
    NoiseTexture(std::shared_ptr<TextureMap3d> texture_map_or_null,
        std::vector<NoiseLayer> layers);
    virtual float evaluate(const DiffGeom& diff_geom) const override final;
  private:
    float evaluate_layer(uint32_t layer_i, Vec3 bary_p) const;
    float evaluate_layer_at_int(uint32_t layer_i, Vec3i bary_p) const;

    static float noise_hash(int32_t x, int32_t y, int32_t z, int32_t w);
  };
}

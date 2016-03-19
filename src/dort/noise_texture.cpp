#include "dort/noise_texture.hpp"
#include "dort/shape.hpp"

namespace dort {
  NoiseTexture::NoiseTexture(
      std::shared_ptr<TextureMap3d> texture_map_or_null,
      std::vector<NoiseLayer> layers):
    texture_map_or_null(texture_map_or_null), layers(std::move(layers))
  { 
    float sum_weight = 0.f;
    for(const auto& layer: this->layers) {
      sum_weight += layer.weight;
    }

    float inv_sum_weight = 1.f / sum_weight;
    for(auto& layer: this->layers) {
      layer.weight *= inv_sum_weight;
      layer.scale *= 0.5f;
    }
  }

  float NoiseTexture::evaluate(const DiffGeom& diff_geom) const {
    Vec3 p = this->texture_map_or_null
      ? this->texture_map_or_null->map(diff_geom)
      : diff_geom.p.v;
    Vec3 bary(
        (-p.x + p.y + p.z),
        ( p.x - p.y + p.z),
        ( p.x + p.y - p.z));

    float f = 0.f;
    for(uint32_t i = 0; i < this->layers.size(); ++i) {
      const auto& layer = this->layers.at(i);
      f += this->evaluate_layer(i, bary * layer.scale) * layer.weight;
    }
    return f;
  }

  float NoiseTexture::evaluate_layer(uint32_t layer_i, Vec3 bary_p) const {
    /*
    Vec3i b0(floor_vec3i(bary_p));
    float f0 = this->evaluate_layer_at_int(layer_i, b0);
    float f1 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(1, 0, 0));
    float f2 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(0, 1, 0));
    float f3 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(0, 0, 1));

    auto fr = [](float a) { return a - floor(a); };
    Vec3 frac(fr(bary_p.x), fr(bary_p.y), fr(bary_p.z));
    return f1 * frac.x + f2 * frac.y + f3 * frac.z +
      f0 * (1.f - frac.x - frac.y - frac.z);
      */

    Vec3i b0(floor_vec3i(bary_p));
    float f000 = this->evaluate_layer_at_int(layer_i, b0);
    float f100 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(1, 0, 0));
    float f010 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(0, 1, 0));
    float f001 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(0, 0, 1));
    float f011 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(0, 1, 1));
    float f101 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(1, 0, 1));
    float f110 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(1, 1, 0));
    float f111 = this->evaluate_layer_at_int(layer_i, b0 + Vec3i(1, 1, 1));

    auto fr = [](float a) { return a - floor(a); };
    Vec3 frac(fr(bary_p.x), fr(bary_p.y), fr(bary_p.z));

    float f00 = lerp(frac.z, f001, f000);
    float f01 = lerp(frac.z, f011, f010);
    float f10 = lerp(frac.z, f101, f100);
    float f11 = lerp(frac.z, f111, f110);
    float f0 = lerp(frac.y, f01, f00);
    float f1 = lerp(frac.y, f11, f10);
    float f = lerp(frac.x, f1, f0);
    return f;
  }

  float NoiseTexture::evaluate_layer_at_int(uint32_t layer_i, Vec3i bary_p) const {
    return noise_hash(bary_p.x, bary_p.y, bary_p.z, layer_i);
  }

  float NoiseTexture::noise_hash(int32_t x, int32_t y, int32_t z, int32_t w) {
    int32_t a = ((x << 11) ^ y + 99367 * (z << 9) ^ w);
    int32_t b = ((y << 7) ^ z + 94649 * (w << 13) ^ x);
    int32_t c = ((z << 13) ^ w + 92377 * (x << 7) ^ y);
    int32_t d = ((w << 12) ^ x + 87613 * (y << 8) ^ z);

    int32_t m = (a * c * 100271);
    int32_t n = (b * d * 103217);
    int32_t o = (a * d * 99623);
    int32_t p = (b * c * 104471);
    
    return abs(((m + n) ^ (o + p)) / float(0xffffffff));
  }
}

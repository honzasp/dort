#include "dort/noise_texture.hpp"
#include "dort/vec_2i.hpp"
#include "dort/vec_3i.hpp"

namespace dort {
  template<class Out, class In>
  Out value_noise(const std::vector<ValueNoiseLayer>& layers, In x) {
    Out value{};
    for(uint32_t i = 0; i < layers.size(); ++i) {
      auto& layer = layers.at(i);
      value = value + value_noise_layer<Out>(i, x * layer.frequency) * layer.weight;
    }
    return value;
  }

  template float value_noise(const std::vector<ValueNoiseLayer>& layers, float x);
  template float value_noise(const std::vector<ValueNoiseLayer>& layers, Vec2 x);
  template float value_noise(const std::vector<ValueNoiseLayer>& layers, Vec3 x);
  template Vec2 value_noise(const std::vector<ValueNoiseLayer>& layers, float x);
  template Vec2 value_noise(const std::vector<ValueNoiseLayer>& layers, Vec2 x);
  template Vec2 value_noise(const std::vector<ValueNoiseLayer>& layers, Vec3 x);
  template Vec3 value_noise(const std::vector<ValueNoiseLayer>& layers, float x);
  template Vec3 value_noise(const std::vector<ValueNoiseLayer>& layers, Vec2 x);
  template Vec3 value_noise(const std::vector<ValueNoiseLayer>& layers, Vec3 x);

  template<class Out> Out value_noise_layer(int32_t layer, float x) {
    int32_t ix = floor_int32(x);
    float dx = x - floor(x);

    Out f0 = value_noise_lattice<Out>(layer, ix);
    Out f1 = value_noise_lattice<Out>(layer, ix + 1);
    Out f = value_noise_interp(dx, f0, f1);
    return f;
  }

  template<class Out> Out value_noise_layer(int32_t layer, Vec2 x) {
    Vec2i ix = floor_vec2i(x);
    Vec2 dx = x - floor(x);
    Out f00 = value_noise_lattice<Out>(layer, ix);
    Out f01 = value_noise_lattice<Out>(layer, ix + Vec2i(0, 1));
    Out f10 = value_noise_lattice<Out>(layer, ix + Vec2i(1, 0));
    Out f11 = value_noise_lattice<Out>(layer, ix + Vec2i(1, 1));
    Out f0 = value_noise_interp(dx[0], f00, f10);
    Out f1 = value_noise_interp(dx[0], f01, f11);
    Out f = value_noise_interp(dx[1], f0, f1);
    return f;
  }

  template<class Out> Out value_noise_layer(int32_t layer, Vec3 x) {
    Vec3i ix = floor_vec3i(x);
    Vec3 dx = x - floor(x);
    Out f000 = value_noise_lattice<Out>(layer, ix);
    Out f001 = value_noise_lattice<Out>(layer, ix + Vec3i(0, 0, 1));
    Out f010 = value_noise_lattice<Out>(layer, ix + Vec3i(0, 1, 0));
    Out f100 = value_noise_lattice<Out>(layer, ix + Vec3i(1, 0, 0));
    Out f011 = value_noise_lattice<Out>(layer, ix + Vec3i(0, 1, 1));
    Out f101 = value_noise_lattice<Out>(layer, ix + Vec3i(1, 0, 1));
    Out f110 = value_noise_lattice<Out>(layer, ix + Vec3i(1, 1, 0));
    Out f111 = value_noise_lattice<Out>(layer, ix + Vec3i(1, 1, 1));
    Out f00 = value_noise_interp(dx[2], f000, f001);
    Out f01 = value_noise_interp(dx[2], f010, f011);
    Out f10 = value_noise_interp(dx[2], f100, f101);
    Out f11 = value_noise_interp(dx[2], f110, f111);
    Out f0 = value_noise_interp(dx[1], f00, f01);
    Out f1 = value_noise_interp(dx[1], f10, f11);
    Out f = value_noise_interp(dx[0], f0, f1);
    return f;
  }

  template<> float value_noise_lattice(int32_t layer, int32_t x) {
    return value_noise_hash(layer, x, 0, 0);
  }
  template<> float value_noise_lattice(int32_t layer, Vec2i x) {
    return value_noise_hash(layer, x[0], x[1], 0);
  }
  template<> float value_noise_lattice(int32_t layer, Vec3i x) {
    return value_noise_hash(layer, x[0], x[1], x[2]);
  }

  template<> Vec2 value_noise_lattice(int32_t layer, int32_t x) {
    return Vec2(
        value_noise_hash(layer, x, 0, 0),
        value_noise_hash(layer, x, 1, 0));
  }
  template<> Vec2 value_noise_lattice(int32_t layer, Vec2i x) {
    return Vec2(
        value_noise_hash(layer, x[0], x[1], 0),
        value_noise_hash(layer, x[0], x[1], 1));
  }
  template<> Vec2 value_noise_lattice(int32_t layer, Vec3i x) {
    return Vec2(
        value_noise_hash(layer, x[0], x[1], x[2]),
        value_noise_hash(layer + 12345, x[0], x[1], x[2]));
  }

  template<> Vec3 value_noise_lattice(int32_t layer, int32_t x) {
    return Vec3(
        value_noise_hash(layer, x, 0, 0),
        value_noise_hash(layer, x, 1, 0),
        value_noise_hash(layer, x, 2, 0));
  }
  template<> Vec3 value_noise_lattice(int32_t layer, Vec2i x) {
    return Vec3(
        value_noise_hash(layer, x[0], x[1], 0),
        value_noise_hash(layer, x[0], x[1], 1),
        value_noise_hash(layer, x[0], x[1], 2));
  }
  template<> Vec3 value_noise_lattice(int32_t layer, Vec3i x) {
    return Vec3(
        value_noise_hash(layer, x[0], x[1], x[2]),
        value_noise_hash(layer + 12345, x[0], x[1], x[2]),
        value_noise_hash(layer + 123456, x[0], x[1], x[2]));
  }

  float value_noise_hash(int32_t x, int32_t y, int32_t z, int32_t w) {
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

  template<class T>
  T value_noise_interp(float x, T t0, T t1) {
    return t0 + 3.f*x*x*(t1 - t0) + 2.f*x*x*x*(t0 - t1);
  }
}

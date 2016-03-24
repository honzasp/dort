#include "dort/noise_texture.hpp"

namespace dort {
  template<class Out, class In>
  std::shared_ptr<Texture<Out, In>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers)
  {
    float sum_weights = 0.f;
    for(auto& layer: layers) {
      sum_weights += layer.weight;
    }
    for(auto& layer: layers) {
      layer.weight *= 1.f / sum_weights;
    }
    return make_texture<Out, In>([=](In x) {
      return value_noise<Out, In>(layers, x);
    });
  }

  template std::shared_ptr<Texture<float, float>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<float, Vec2>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<float, Vec3>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);

  template std::shared_ptr<Texture<Vec2, float>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<Vec2, Vec2>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<Vec2, Vec3>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);

  template std::shared_ptr<Texture<Vec3, float>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<Vec3, Vec2>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
  template std::shared_ptr<Texture<Vec3, Vec3>> value_noise_texture(
      std::vector<ValueNoiseLayer> layers);
}

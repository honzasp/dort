#pragma once
#include "dort/bsdf.hpp"
#include "dort/discrete_distrib_1d.hpp"
#include "dort/light.hpp"
#include "dort/sample_renderer.hpp"
#include "dort/slice.hpp"

namespace dort {
  /// A bidirectional path renderer.
  /// This renderer samples vertices both from light and the camera and combines
  /// the various sub-paths using multiple importance sampling.
  class BdptRenderer final: public SampleRenderer {
    uint32_t max_depth;
    DiscreteDistrib1d light_distrib;
  public:
    BdptRenderer(std::shared_ptr<Scene> scene,
        std::shared_ptr<Film> film,
        std::shared_ptr<Sampler> sampler,
        uint32_t max_depth):
      SampleRenderer(scene, film, sampler), max_depth(max_depth) 
    { }
    virtual Spectrum get_radiance(const Scene& scene, Ray& ray,
        uint32_t depth, Sampler& sampler) const override final;
  private:
    virtual void preprocess(CtxG& ctx,
        const Scene& scene, Sampler& sampler) override final;

    // The unweighted contribution of a path x[0], ..., x[n] is
    //
    // C* = f(x[0..n]) / p(x[0..n])
    // where
    // f(x[0..n]) =
    //   Le(x[0] -> x[1]) * We(x[n-1], x[n]) *
    //   product([G(x[i] <-> x[i+1]) for i in 0..n-1]) *
    //   product([bsdf(x[i] -> x[i+1] -> x[i+2] for i in 0..n-2])
    // p(x[0..n]) =
    //   product([p(x[i]) for i in 0..n])
    //
    // For efficient connection of light path y[0], ..., y[s-1] and camera path
    // z[0], ..., z[t-1], we cache the quantity alpha(x[i]) for each vertex i,
    // such that
    //
    // C*(y[0..s-1] z[0..t-1]) =
    //   f(y[0..s-1] z[0..t-1]) / p(y[0..s-1] z[0..t-1]) = 
    //   alpha(y[s-1]) * c(s,t) * alpha(z[t-1])
    // where
    // c(s,t) = G(y[s-1] <-> z[t-1]) *
    //   bsdf(y[s-2] -> y[s-1] -> z[t-1]) *
    //   bsdf(y[s-1] -> z[t-1] -> z[t-2])
    //
    // Therefore, alpha(x[n]) is defined as:
    // alpha(x[n]) = product([rho(x[i]) / p(x[i]) for i in 0..n])
    // where
    // rho(y[0]) = Le0(y[0])
    // rho(y[1]) = Le1(y[1])
    // rho(y[i+2]) = bsdf(y[i] -> y[i+1] -> y[i+2])
    // rho(z[0]) = We0(z[0])
    // rho(z[1]) = We1(z[1])
    // rho(z[i+2]) = bsdf(z[i+2] -> z[i+1] -> z[i])
    //
    // p(x[0]) = pArea(x[0])
    // p(x[i+1]) = pDir(x[i+1] <- x[i]) / abs(n[i] dot (x[i] -> x[i+1]))
    struct Vertex {
      Point p;
      Normal nn;
      std::unique_ptr<Bsdf> bsdf;
      const AreaLight* area_light;
      float p_epsilon;
      float fwd_pdf;
      float bwd_pdf;
      float bwd_geom;
      Spectrum alpha;
    };

    std::vector<Vertex> random_light_walk(const Scene& scene, Rng& rng) const;
    std::vector<Vertex> random_camera_walk(const Scene& scene,
        const Ray& ray, Rng& rng) const;
    Spectrum path_contrib(const Scene& scene, Vec2 film_res, Rng& rng,
        const std::vector<Vertex>& light_walk,
        const std::vector<Vertex>& camera_walk,
        uint32_t s, uint32_t t, Vec2& out_film_pos) const;
    float path_weight(const std::vector<Vertex>& light_walk,
        const std::vector<Vertex>& camera_walk,
        uint32_t s, uint32_t t) const;
  };
}

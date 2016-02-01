#pragma once
#include "dort/geometry.hpp"
#include "dort/scene.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  struct ShadowTest {
    Ray ray;

    ShadowTest(): ray(Point(), Vector()) { }
    void init_point_point(const Point& p1, float epsilon_1,
        const Point& p2, float epsilon_2);
    void init_point_dir(const Point& pt, float epsilon,
        const Vector& dir);
    bool visible(const Scene& scene) const;
  };

  class Light {
  public:
    virtual ~Light() {}
    virtual Spectrum sample_radiance(const Point& pt, float pt_epsilon,
        Vector& out_wi, float& out_pdf, ShadowTest& out_shadow) const = 0;
    virtual Spectrum background_radiance(const Ray& ray) const;
  };
}

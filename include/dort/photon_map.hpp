#pragma once
#include <vector>
#include "dort/geometry.hpp"
#include "dort/kd_tree.hpp"
#include "dort/spectrum.hpp"

namespace dort {
  struct Photon final {
    Spectrum power;
    Point p;
    Vector wi;
    Normal nn;
  };

  class PhotonMap final {
    struct KdTraits {
      using Element = Photon;
      static Point element_point(const Element& elem) {
        return elem.p;
      }
    };

    KdTree<KdTraits> photon_tree;
    uint32_t emitted_count;
  public:
    PhotonMap() = default;
    PhotonMap(std::vector<Photon> photons, uint32_t emitted_count);
    Spectrum estimate_radiance(const Point& p, const Normal& nn,
        const Vector& wo, const Bsdf& bsdf, float radius) const;
  };
}

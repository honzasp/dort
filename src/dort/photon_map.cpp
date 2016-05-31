#include "dort/bsdf.hpp"
#include "dort/photon_map.hpp"

namespace dort {
  PhotonMap::PhotonMap(std::vector<Photon> photons,
      uint32_t emitted_count):
    photon_tree(std::move(photons)),
    emitted_count(emitted_count)
  { }

  Spectrum PhotonMap::estimate_radiance(const Point& p, const Normal& nn,
      const Vector& wo, const Bsdf& bsdf, float radius) const
  {
    (void)nn;
    Spectrum power(0.f);
    this->photon_tree.lookup(p, square(radius),
      [&](const Photon& photon, float dist_square, float radius_square) {
        (void)dist_square;
        if(dot(photon.nn, nn) >= 0.7f) {
          power += photon.power * bsdf.f(wo, photon.wi, BSDF_ALL);
        }
        return radius_square;
      });
    return power / (float(this->emitted_count) * PI * square(radius));
  }
}

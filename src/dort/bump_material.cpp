#include "dort/bump_material.hpp"
#include "dort/shape.hpp"

namespace dort {
  void BumpMaterial::add_bxdfs(const DiffGeom& geom, Spectrum scale, Bsdf& bsdf) const {
    // TODO: base the deltas on image-space distances!
    float delta_u = 0.1f;
    float delta_v = 0.1f;

    float displac = this->displac->evaluate(geom);

    DiffGeom tmp_geom(geom);
    tmp_geom.uv = geom.uv + Vec2(delta_u, 0.f);
    tmp_geom.p = geom.p + delta_u * geom.dpdu_shading;
    // TODO: shift the normal by delta_u * dndu!
    tmp_geom.nn = geom.nn_shading; 
    float displac_u = this->displac->evaluate(tmp_geom);

    tmp_geom.uv = geom.uv + Vec2(0.f, delta_v);
    tmp_geom.p = geom.p + delta_u * geom.dpdu_shading;
    tmp_geom.nn = geom.nn_shading;
    float displac_v = this->displac->evaluate(tmp_geom);

    float dddu = (displac_u - displac) / delta_u;
    float dddv = (displac_v - displac) / delta_v;

    DiffGeom bump_geom(geom);
    // TODO: add dndu * displac!
    bump_geom.dpdu_shading = geom.dpdu_shading 
      + dddu * Vector(geom.nn_shading); 
    bump_geom.dpdv_shading = geom.dpdv_shading
      + dddv * Vector(geom.nn_shading);
    bump_geom.nn_shading = Normal(normalize(cross(
      bump_geom.dpdu_shading, bump_geom.dpdv_shading)));
    if(dot(bump_geom.nn_shading, bump_geom.nn) < 0.f) {
      bump_geom.nn_shading = -bump_geom.nn_shading;
    }

    this->material->add_bxdfs(bump_geom, scale, bsdf);
  }
}

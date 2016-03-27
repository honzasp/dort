#include "dort/bsdf.hpp"
#include "dort/bump_material.hpp"
#include "dort/shape.hpp"
#include "dort/texture.hpp"

namespace dort {
  std::unique_ptr<Bsdf> BumpMaterial::get_bsdf(
      const DiffGeom& shading_geom, const Normal& nn_geom) const
  {
    // TODO: base the deltas on image-space distances!
    float delta_u = 0.1f;
    float delta_v = 0.1f;

    float displac = this->displac->evaluate(shading_geom);

    DiffGeom tmp_geom(shading_geom);
    tmp_geom.u = shading_geom.u + delta_u;
    tmp_geom.v = shading_geom.v;
    tmp_geom.p = shading_geom.p + delta_u * shading_geom.dpdu;
    // TODO: shift the normal by delta_u * dndu!
    tmp_geom.nn = shading_geom.nn; 
    float displac_u = this->displac->evaluate(tmp_geom);

    tmp_geom.u = shading_geom.u;
    tmp_geom.v = shading_geom.v + delta_v;
    tmp_geom.p = shading_geom.p + delta_u * shading_geom.dpdu;
    float displac_v = this->displac->evaluate(tmp_geom);

    float dddu = (displac_u - displac) / delta_u;
    float dddv = (displac_v - displac) / delta_v;

    DiffGeom bump_geom(shading_geom);
    // TODO: add dndu * displac!
    bump_geom.dpdu = shading_geom.dpdu + dddu * Vector(shading_geom.nn); 
    bump_geom.dpdv = shading_geom.dpdv + dddv * Vector(shading_geom.nn);
    bump_geom.nn = Normal(normalize(cross(bump_geom.dpdu, bump_geom.dpdv)));
    if(dot(bump_geom.nn, shading_geom.nn) < 0.f) {
      bump_geom.nn = -bump_geom.nn;
    }

    return this->material->get_bsdf(bump_geom, nn_geom);
  }
}

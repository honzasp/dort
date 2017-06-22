/// Lights.
// @module dort.light
#include "dort/beam_light.hpp"
#include "dort/diffuse_light.hpp"
#include "dort/directional_light.hpp"
#include "dort/environment_light.hpp"
#include "dort/infinite_light.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_params.hpp"
#include "dort/point_light.hpp"

namespace dort {
  int lua_open_light(lua_State* l) {
    const luaL_Reg light_methods[] = {
      {"__gc", lua_gc_shared_obj<Light, LIGHT_TNAME>},
      {0, 0},
    };

    const luaL_Reg light_funs[] = {
      {"make_point", lua_light_make_point},
      {"make_directional", lua_light_make_directional},
      {"make_diffuse", lua_light_make_diffuse},
      {"make_infinite", lua_light_make_infinite},
      {"make_environment", lua_light_make_environment},
      {"make_beam", lua_light_make_beam},
      {0, 0},
    };

    lua_register_type(l, LIGHT_TNAME, light_methods);
    luaL_newlib(l, light_funs);
    return 1;
  }

  /// Make a point light.
  //
  // - `point` -- the location (`Point`) of the light
  // - `intensity` -- intensity (`Spectrum`)
  // - `transform` -- the light-to-world transform (identity by default)
  //
  // @function make_point
  // @param params
  int lua_light_make_point(lua_State* l) {
    int p = 1;
    auto point = lua_param_point(l, p, "point");
    auto intensity = lua_param_spectrum(l, p, "intensity");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<PointLight>(transform.apply(point), intensity));
    return 1;
  }

  /// Make a directional light.
  //
  // - `direction` -- the direction (`Vector`) of the light
  // - `radiance` -- the emitted radiance
  // - `transform` -- the light-to-world transform (identity by default)
  //
  // @function make_directional
  // @param params
  int lua_light_make_directional(lua_State* l) {
    int p = 1;
    auto direction = lua_param_vector(l, p, "direction");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<DirectionalLight>(
          transform.apply(direction), radiance));
    return 1;
  }

  /// Make a diffuse area light.
  //
  // - `shape` -- the `Shape` of the area light
  // - `radiance` -- the radiance emitted in all directions from each point of
  //   the `shape`.
  // - `transform` -- the light-to-world transform (identity by default)
  //
  // Note that when using area lights, the scene must contain both the light and
  // the shape linked to this light, otherwise some lighting may be lost.
  // @function make_diffuse
  // @param params
  int lua_light_make_diffuse(lua_State* l) {
    int p = 1;
    auto shape = lua_param_shape(l, p, "shape");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<DiffuseLight>(
          shape, transform, radiance));
    return 1;
  }

  /// Make a homogeneous infinite area light.
  //
  // - `radiance` -- the radiance incident at every point in the scene from the
  //   infinite sphere surrounding the scene.
  //
  // @function make_infinite
  // @param params
  int lua_light_make_infinite(lua_State* l) {
    int p = 1;
    auto radiance = lua_param_spectrum(l, p, "radiance");
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<InfiniteLight>(radiance));
    return 1;
  }

  /// Make an image-mapped infinite area light.
  //
  // - `image` -- the `Image.RgbFloat`, a HDR image mapped to the infinite
  // sphere surrounding the scene.
  // - `up` -- the `Vector` that determines the "up" direction for the sphere
  // - `forward` -- the `Vector` that determines the "forward" direction for the
  // sphere.
  // - `scale` -- an optional `Spectrum` to scale the values from the `image` to
  // radiance
  // - `transform` -- the light-to-world transform (identity by default).
  //
  // @function make_environment
  // @param params
  int lua_light_make_environment(lua_State* l) {
    int p = 1;
    auto image = lua_param_image_f(l, p, "image");
    auto up = lua_param_vector(l, p, "up");
    auto forward = lua_param_vector(l, p, "forward");
    auto scale = lua_param_spectrum_opt(l, p, "scale", Spectrum(1.f));
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<EnvironmentLight>(image, scale,
          transform.apply(up), transform.apply(forward)));
    return 1;
  }

  /// Make a single beam of light.
  //
  // - `point` -- the origin of the beam
  // - `direction` -- the direction of the beam
  // - `radiance` -- the radiance of the beam
  // - `transform` -- the light-to-world transform
  //
  // @function make_beam
  // @param params
  int lua_light_make_beam(lua_State* l) {
    int p = 1;
    auto pt = lua_param_point(l, p, "point");
    auto dir = lua_param_vector(l, p, "direction");
    auto radiance = lua_param_spectrum(l, p, "radiance");
    auto transform = lua_param_transform_opt(l, p, "transform", identity());
    lua_params_check_unused(l, p);

    lua_push_light(l, std::make_shared<BeamLight>(
          transform.apply(pt), transform.apply(dir), radiance));
    return 1;
  }

  std::shared_ptr<Light> lua_check_light(lua_State* l, int idx) {
    return lua_check_shared_obj<Light, LIGHT_TNAME>(l, idx);
  }
  bool lua_test_light(lua_State* l, int idx) {
    return lua_test_shared_obj<Light, LIGHT_TNAME>(l, idx);
  }
  void lua_push_light(lua_State* l, std::shared_ptr<Light> light) {
    lua_push_shared_obj<Light, LIGHT_TNAME>(l, light);
  }
}

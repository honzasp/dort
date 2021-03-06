/// Raster images.
// There are two types of images:
//
// - `Image.Rgb8` -- low dynamic range image, uses 8-bit integers for each
//   channel.
// - `Image.RgbFloat` -- high dynamic range images, uses 32-bit floats for each
//   channel.
//
// @module dort.image
#include "dort/convergence_test.hpp"
#include "dort/image.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_helpers.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_params.hpp"
#include "dort/rect_i.hpp"
#include "dort/tonemap.hpp"

namespace dort {
  int lua_open_image(lua_State* l) {
    const luaL_Reg image8_methods[] = {
      {"__gc", lua_gc_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>},
      {0, 0},
    };

    const luaL_Reg imagef_methods[] = {
      {"__gc", lua_gc_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>},
      {0, 0},
    };

    const luaL_Reg image_funs[] = {
      {"read", lua_image_read},
      {"write_png", lua_image_write_png},
      {"write_rgbe", lua_image_write_rgbe},
      {"tonemap_srgb", lua_image_tonemap_srgb},
      {"tonemap_gamma", lua_image_tonemap_gamma},
      {"get_res", lua_image_get_res},
      {"bias_variance", lua_image_bias_variance},
      {"test_convergence", lua_image_test_convergence},
      {0, 0},
    };

    lua_register_type(l, IMAGE_RGB8_TNAME, image8_methods);
    lua_register_type(l, IMAGE_RGBF_TNAME, imagef_methods);
    luaL_newlib(l, image_funs);
    return 1;
  }

  /// Reads an image from file.
  // Opens the file `file_name` and reads the image in the file. `params` are:
  //
  // - `hdr` -- if true, reads `Image.RgbFloat`, otherwise reads `Image.Rgb8`.
  //
  // @function read
  // @param file_name
  // @param params
  int lua_image_read(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    int p = 2;
    bool hdr = lua_param_bool_opt(l, p, "hdr", false);
    lua_params_check_unused(l, p);

    FILE* file = std::fopen(file_name, "r");
    if(!file) {
      return luaL_error(l, "Could not open image file for reading: %s", file_name);
    }

    if(hdr) {
      auto image = std::make_shared<Image<PixelRgbFloat>>(read_image_f(file));
      lua_push_image_f(l, image);
    } else {
      auto image = std::make_shared<Image<PixelRgb8>>(read_image_8(file));
      lua_push_image_8(l, image);
    }

    std::fclose(file);
    return 1;
  }

  /// Write an `Image.Rgb8` as PNG.
  // @function write_png
  // @param file_name
  // @param image
  int lua_image_write_png(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    auto image = lua_check_image_8(l, 2);
    FILE* file = std::fopen(file_name, "w");
    if(!file) {
      lua_pushfstring(l, "Could not open file for writing: %s", file_name);
      lua_error(l);
      return 0;
    }
    write_image_png(file, *image);
    std::fclose(file);
    return 0;
  }

  /// Write an `Image.RgbFloat` as Radiance HDR file.
  // @function write_rgbe
  // @param file_name
  // @param image
  int lua_image_write_rgbe(lua_State* l) {
    const char* file_name = luaL_checkstring(l, 1);
    auto image = lua_check_image_f(l, 2);
    FILE* file = std::fopen(file_name, "w");
    if(!file) {
      lua_pushfstring(l, "Could not open file for writing: %s", file_name);
      lua_error(l);
      return 0;
    }
    write_image_rgbe(file, *image);
    std::fclose(file);
    return 0;
  }

  /// Tonemap using the sRGB curve.
  // Maps the input `Image.RgbFloat` into an `Image.Rgb8` using the sRGB curve.
  // The image is premultiplied by `scale` (by default 1).
  // @function tonemap_srgb
  // @param image
  // @param[opt] scale
  int lua_image_tonemap_srgb(lua_State* l) {
    auto image = lua_check_image_f(l, 1);
    float scale = lua_gettop(l) >= 2 ? luaL_checknumber(l, 2) : 1.0;
    auto out_image = tonemap_srgb(*image, scale);
    lua_push_image_8(l, std::make_shared<Image<PixelRgb8>>(std::move(out_image)));
    return 1;
  }

  /// Tonemap using a gamma curve.
  // Maps the input `Image.RgbFloat` into an `Image.Rgb8` using the gamma
  // (exponential) curve with exponent `1/gamma`. The input image is optionally
  // premultiplied by `scale`.
  // @function tonemap_gamma
  // @param image
  // @param[opt] gamma
  // @param[opt] scale
  int lua_image_tonemap_gamma(lua_State* l) {
    auto image = lua_check_image_f(l, 1);
    float gamma = lua_gettop(l) >= 2 ? luaL_checknumber(l, 2) : 1.0;
    float scale = lua_gettop(l) >= 3 ? luaL_checknumber(l, 3) : 1.0;
    auto out_image = tonemap_gamma(*image, gamma, scale);
    lua_push_image_8(l, std::make_shared<Image<PixelRgb8>>(std::move(out_image)));
    return 1;
  }

  /// Get the resolution of the image.
  // Returns the resolution as two return values `x` and `y`.
  // @function get_res
  // @param image
  int lua_image_get_res(lua_State* l) {
    uint32_t x_res, y_res;
    if(lua_test_image_8(l, 1)) {
      auto image = lua_check_image_8(l, 1);
      x_res = image->res.x;
      y_res = image->res.y;
    } else if(lua_test_image_f(l, 1)) {
      auto image = lua_check_image_f(l, 1);
      x_res = image->res.x;
      y_res = image->res.y;
    } else {
      return luaL_error(l, "Expected an image");
    }

    lua_pushinteger(l, x_res);
    lua_pushinteger(l, y_res);
    return 2;
  }

  /// Computes bias and variance of a `tested` image w.r.t. a `reference`.
  // Computes the sum of `abs(p_ref - p_tested)` (bias) and the sum of `(p_ref -
  // p_tested)^2` (variance), where `p_ref` and `p_tested` are the pixels of
  // `reference` and `tested` in the rectangle `rect`. Returns the bias and
  // variance as two return values.
  // @function bias_variance
  // @param reference
  // @param tested
  // @param rect
  int lua_image_bias_variance(lua_State* l) {
    auto ref_image = lua_check_image_f(l, 1);
    auto tested_image = lua_check_image_f(l, 2);
    Recti rect = lua_check_recti(l, 3);

    uint32_t x0 = rect.p_min.x;
    uint32_t x1 = rect.p_max.x;
    uint32_t y0 = rect.p_min.y;
    uint32_t y1 = rect.p_max.y;

    if(x1 > uint32_t(ref_image->res.x) || y1 > uint32_t(ref_image->res.y)) {
      return luaL_error(l, "Rect is out of the ref_image");
    }
    if(x1 > uint32_t(tested_image->res.x) || y1 > uint32_t(tested_image->res.y)) {
      return luaL_error(l, "Rect is out of the tested_image");
    }

    float sum_bias = 0.f;
    float sum_var = 0.f;
    for(uint32_t y = y0; y < y1; ++y) {
      for(uint32_t x = x0; x < x1; ++x) {
        auto ref_rgb = PixelRgbFloat::to_rgb(ref_image->get_pixel(x, y));
        auto tested_rgb = PixelRgbFloat::to_rgb(tested_image->get_pixel(x, y));
        auto delta = (ref_rgb - tested_rgb).rgb;
        sum_bias += delta.sum();
        sum_var += (delta * delta).sum();
      }
    }

    lua_pushnumber(l, sum_bias);
    lua_pushnumber(l, sum_var);
    return 2;
  }

  /// Applies a convergence test to `tested` image w.r.t. `reference`.
  //
  // The images are decomposed into tiles of various sizes and a simple
  // statistical test is applied to each tile to determine whether the
  // difference between `tested` and `reference` is only due to random noise.
  // The `params` are:
  //
  // - `min_tile_size` -- minimum size of a tested tile.
  // - `variation` -- variation of the reference image (`stddev / mean`).
  // - `bias` -- maximum amount of tolerated bias, relative to the mean value (0
  //   by default).
  // - `p_value` -- desired probability that the test passes even though the
  //   images are different.
  //
  // @function test_convergence
  // @param reference
  // @param tested
  // @param params
  int lua_image_test_convergence(lua_State* l) {
    int p = 3;
    auto ref_image = lua_check_image_f(l, 1);
    auto tested_image = lua_check_image_f(l, 2);
    int32_t min_tile_size = lua_param_uint32(l, p, "min_tile_size");
    float variation = lua_param_float(l, p, "variation");
    float bias = lua_param_float_opt(l, p, "bias", 0.f);
    float p_value = lua_param_float(l, p, "p_value");
    lua_params_check_unused(l, p);

    if(!(ref_image->res == tested_image->res)) {
      return luaL_error(l, "Reference and test images have mismatched resolution");
    }

    auto error = test_convergence(*lua_get_ctx(l), *ref_image, *tested_image,
        min_tile_size, variation, bias, p_value);
    if(error.empty()) {
      lua_pushnil(l);
    } else {
      lua_pushlstring(l, error.data(), error.size());
    }
    return 1;
  }

  std::shared_ptr<Image<PixelRgb8>> lua_check_image_8(lua_State* l, int idx) {
    return lua_check_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  bool lua_test_image_8(lua_State* l, int idx) {
    return lua_test_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, idx);
  }
  void lua_push_image_8(lua_State* l, std::shared_ptr<Image<PixelRgb8>> image) {
    return lua_push_shared_obj<Image<PixelRgb8>, IMAGE_RGB8_TNAME>(l, std::move(image));
  }

  std::shared_ptr<Image<PixelRgbFloat>> lua_check_image_f(lua_State* l, int idx) {
    return lua_check_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(l, idx);
  }
  bool lua_test_image_f(lua_State* l, int idx) {
    return lua_test_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(l, idx);
  }
  void lua_push_image_f(lua_State* l, std::shared_ptr<Image<PixelRgbFloat>> image) {
    return lua_push_shared_obj<Image<PixelRgbFloat>, IMAGE_RGBF_TNAME>(
        l, std::move(image));
  }
}

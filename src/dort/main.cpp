#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "dort/geometry.hpp"
#include "dort/lua_builder.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_light.hpp"
#include "dort/lua_material.hpp"
#include "dort/lua_math.hpp"
#include "dort/lua_shape.hpp"
#include "dort/lua_texture.hpp"

namespace dort {
  int lua_answer(lua_State* l) {
    lua_pushinteger(l, 42);
    return 1;
  }

  int main() {
    std::string prog = R"(
      print("vectors")
      local v1 = vector(1, 2, -pi)
      local v2 = vector(-1, 3, 2.3)
      print(v1:z())
      print(v2:y())
      print(v1)
      print(v1 + v2)

      print("points")
      local p1 = point(1, 2, 3)
      print(p1)
      print(p1:y())

      print("math")
      print(sqrt(10))
      print(min(1, -10, infinity))
      print(pow(2, 3))

      print("transforms")
      local t1 = translate(1, 2, -3) * rotate_x(pi)
      print(t1)
      print(t1(p1))
      print(t1(v2))
      print(t1:apply(v1))
      print(t1:apply(false, v1))
      print(t1:inverse():apply(p1))
      print(t1:apply_inv(p1))
      print(t1:apply(true, p1))

      --print("image")
      --local img = read_image("lua.jpg")
      --write_png_image("lua_2.png", img)

      print("texture")
      print(const_texture(0.1))
      print(const_texture(0.1) * 10)
      print(0.23 * const_texture(rgb(1, 0, 1)))
      local tex1 = checkerboard_texture {
        map = xy_texture_map(),
        odd_check = 0.5,
      }
      local tex2 = checkerboard_texture {
        map = cylindrical_texture_map(rotate_x(0.5 * pi)),
        check_size = 0.5,
        odd_check = rgb(1, 0, 0),
        even_check = rgb(0, 0, 1),
      };
      local tex3 = lerp_texture {
        t = tex1, tex_0 = tex2, tex_1 = rgb(1, 0, 1),
      }
      print(tex2 + tex3 * 0.5)
    )";

    lua_State* l = luaL_newstate();
    luaL_requiref(l, "base", luaopen_base, true);
    luaL_requiref(l, MATH_LIBNAME, lua_open_math, true);
    luaL_requiref(l, GEOMETRY_LIBNAME, lua_open_geometry, true);
    luaL_requiref(l, IMAGE_LIBNAME, lua_open_image, true);
    luaL_requiref(l, TEXTURE_LIBNAME, lua_open_texture, true);
    luaL_requiref(l, SHAPE_LIBNAME, lua_open_shape, true);
    luaL_requiref(l, MATERIAL_LIBNAME, lua_open_material, true);
    luaL_requiref(l, LIGHT_LIBNAME, lua_open_light, true);
    luaL_requiref(l, BUILDER_LIBNAME, lua_open_builder, true);

    int error;
    if((error = luaL_loadfile(l, "examples/balls.lua"))) {
      std::fprintf(stderr, "Load error: %s\n", lua_tostring(l, -1));
      lua_pop(l, 1);
      lua_close(l);
      return 1;
    }

    if((error = lua_pcall(l, 0, 0, 0))) {
      std::fprintf(stderr, "Runtime error: %s\n", lua_tostring(l, -1));
      lua_pop(l, 1);
      lua_close(l);
      return 1;
    }

    lua_close(l);
    return 0;
  }
}

int main() {
  return dort::main();
}

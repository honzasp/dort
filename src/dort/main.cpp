#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "dort/geometry.hpp"
#include "dort/lua_geometry.hpp"
#include "dort/lua_image.hpp"
#include "dort/lua_math.hpp"

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

      print("image")
      local img = read_image("lua.jpg")
      write_png_image("lua_2.png", img)
    )";

    lua_State* l = luaL_newstate();
    luaL_requiref(l, "base", luaopen_base, true);
    luaL_requiref(l, MATH_LIBNAME, lua_open_math, true);
    luaL_requiref(l, GEOMETRY_LIBNAME, lua_open_geometry, true);
    luaL_requiref(l, IMAGE_LIBNAME, lua_open_image, true);

    int error;
    if((error = luaL_loadbuffer(l, prog.data(), prog.size(), "hello"))) {
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

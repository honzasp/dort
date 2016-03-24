local _ENV = require "dort/dsl"

local function const_rgb(r, g, b)
  return const_texture_2d(rgb(r, g, b))
end

local tex = 
  checkerboard_texture_2d {
    even = const_rgb(1, 1, 1),
    odd = const_rgb(0, 0, 0),
    check_size = 1 / 8,
  }

write_png_image("texture.png", render_texture_2d(tex, { }))

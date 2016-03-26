local _ENV = require "dort/dsl"

local function const_rgb(r, g, b)
  return const_texture_2d(rgb(r, g, b))
end

local function render(name, tex, opts)
  opts = opts or {}
  write_png_image("tex_" .. name .. ".png", render_texture_2d(tex, opts))
end

--[[
render("noise", compose_texture(
  value_noise_texture_2d {
    { weight = 1, frequency = 512 },
    { weight = 1, frequency = 256 },
    { weight = 2, frequency = 128 },
    { weight = 2, frequency = 64 },
    { weight = 2, frequency = 32 },
    { weight = 1, frequency = 16 },
    { weight = 1, frequency = 8 },
  },
  identity_texture_2d() + const_texture_2d(0.8) * value_noise_texture_2d_of_2d {
    { weight = 1, frequency = 64 },
    { weight = 3, frequency = 32 },
    { weight = 1, frequency = 16 },
    { weight = 4, frequency = 8 },
    { weight = 6, frequency = 4 },
  }
), { x_res = 1366, y_res = 768 })
--]]

render("color_noise", 
  lerp_color_map(rgbh("16b4f6"), rgbh("1940a2")) ..
  gain_texture(0.8) ..
  value_noise_texture_2d {
    { weight = 1, frequency = 512 },
    { weight = 1, frequency = 256 },
    { weight = 2, frequency = 128 },
    { weight = 2, frequency = 64 },
    { weight = 2, frequency = 32 },
    { weight = 1, frequency = 16 },
    { weight = 1, frequency = 8 },
  } ..
  identity_texture_2d() + const_texture_2d(0.8) * value_noise_texture_2d_of_2d {
    { weight = 1, frequency = 64 },
    { weight = 3, frequency = 32 },
    { weight = 1, frequency = 16 },
    { weight = 4, frequency = 8 },
    { weight = 6, frequency = 4 },
  },
  { x_res = 1440, y_res = 900, scale = 1 }
)
